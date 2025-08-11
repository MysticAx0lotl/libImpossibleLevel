#include "libImpossibleLevel.hpp"

//Loads a binary into a vector
static std::vector<unsigned char> ReadAllBytes(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        return std::vector<unsigned char>{};
    }

    return std::vector<unsigned char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

//Java handles evereything in big-Endian
//Since TIG's level editor is written in java, ints and shorts are written as big-Endian
//They must be converted to little-Endian after being read to be useable here
//This function takes an array of chars and a byte to start from. 
//It bit-shifts the starting byte and the next three bytes, then joins them together into a single int
//file = loaded file as a array of chars, startingOffset = the byte to start processing from
int readIntFromJava(std::vector<unsigned char> &file, int startingOffset)
{
    unsigned int bit1, bit2, bit3, bit4;
    bit1 = static_cast<unsigned int>(file[startingOffset]);
    bit2 = static_cast<unsigned int>(file[startingOffset + 1]);
    bit3 = static_cast<unsigned int>(file[startingOffset + 2]);
    bit4 = static_cast<unsigned int>(file[startingOffset + 3]);

    bit1 = bit1 << 24;
    bit2 = bit2 << 16;
    bit3 = bit3 << 8;
    //bit 4 doesn't get shifted

    unsigned int resultU = bit1 | bit2 | bit3 | bit4;
    int result = static_cast<int>(resultU);
    return result;
}

//This function takes an array of chars and a byte to start from. 
//It bit-shifts the starting byte and the next byte, then joins them together into a single short
//file = loaded file as a array of chars, startingOffset = the byte to start processing from
short readShortFromJava(std::vector<unsigned char> &file, int startingOffset)
{
    unsigned short bit1, bit2;
    bit1 = static_cast<unsigned int>(file[startingOffset]);
    bit2 = static_cast<unsigned int>(file[startingOffset + 1]);

    bit1 = bit1 << 8;
    //bit 2 doesn't get shifted

    unsigned short resultU = bit1 | bit2;
    short result = static_cast<short>(resultU);
    return result;
}


//Reads a Java-formatted UTF8-string from a given file
//This starts with a two-byte short determining how long the string is, which is followed by the string itself
std::string readUTF8FromJava(std::vector<unsigned char> &file, int startingOffset)
{
    int currentOffset = startingOffset;
    int strLen = readShortFromJava(file, currentOffset);
    std::string result;

    currentOffset += 2;

    for(int i = currentOffset; i < (currentOffset + strLen); i++)
    {
        result += file[i];
    }

    return result;
}

//Java handles evereything in big-Endian
//Since TIG's level editor is written in java, ints and shorts are read as big-Endian
//They must be converted to little-Endian after being written to be accepted by the game
void writeJavaInt(std::ofstream& datafile, int sourceInt)
{
    unsigned int swapSource = __builtin_bswap32(static_cast<unsigned int>(sourceInt));
    datafile.write(reinterpret_cast<const char*>(&swapSource), sizeof(swapSource));
}

//Java handles evereything in big-Endian
//Since TIG's level editor is written in java, ints and shorts are read as big-Endian
//They must be converted to little-Endian after being written to be accepted by the game
void writeJavaShort(std::ofstream& datafile, short sourceShort)
{
    unsigned short swapSource = static_cast<unsigned short>((sourceShort >> 8) | (sourceShort << 8));
    datafile.write(reinterpret_cast<const char*>(&swapSource), sizeof(swapSource));
}

//Java's UTF8 format appends the string length to the beginning of the data
void writeJavaUTF8(std::ofstream& datafile, std::string sourceStr)
{
    writeJavaShort(datafile, sourceStr.size());
    datafile.write(sourceStr.c_str(), sourceStr.size());
}

//Endianess doesn't matter for bools or char arrays, this function handles exporting those
void writeOtherData(std::ofstream& datafile, unsigned char data)
{
    datafile.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

//source: https://stackoverflow.com/questions/4892680/sorting-a-vector-of-structs
bool compareByPos(const BlockObject &a, const BlockObject &b)
{
    return a.xPos < b.xPos;
}

//Constructor that generates a blank level if no filepath is given
Level::Level(bool debugMode)
{
    if(debugMode){std::cout << "No filepath was given, a blank level will be generated..." << std::endl;}

    this->numBlockObjects = 0;
    this->numBackgroundChanges = 0;
    this->numGravityChanges = 0;
    this->numBlocksRise = 0;
    this->numBlocksFall = 0;
    this->endPos = 3015;
    this->customGraphicsEnabled = false;
    this->formatVer = 0;

    if(debugMode){std::cout << "Blank level generated!" << std::endl;}
}

//Constructor that calls loadDataFromFile
Level::Level(std::string filename, bool debugMode)
{
    filename += this->tig_filepath;
    if(std::filesystem::exists(filename))
    {
        if(debugMode){std::cout << "Attempting to read file " << filename.c_str() << std::endl;}
        std::vector<unsigned char> levelChars = ReadAllBytes(filename.c_str()); //load file from path
        this->loadLevel(levelChars, debugMode);
    }
    else
    {
        if(debugMode){std::cout << filename.c_str() << " does not seem to exist. Generating a blank level instead..." << std::endl;}
        
        //copied verbatim from Level::Level(bool)
        this->numBlockObjects = 0;
        this->numBackgroundChanges = 0;
        this->numGravityChanges = 0;
        this->numBlocksRise = 0;
        this->numBlocksFall = 0;
        this->endPos = 3015;
        this->customGraphicsEnabled = false;
        this->formatVer = 0;
    
        if(debugMode){std::cout << "Blank level generated!" << std::endl;}
        loadedSuccessfully = true;
    }
}

Level::Level(std::vector<unsigned char> levelChars, bool debugMode)
{
    if(debugMode){std::cout << "Attempting to read C char array" << std::endl;}
    this->loadLevel(levelChars, debugMode);
}

//Destructor that serves no purpose right now
Level::~Level()
{

}

//Parse level data from the given filepath, called by constructor
void Level::loadLevel(std::vector<unsigned char> levelChars, bool debugMode)
{
    int currentByte = 0; //tracks the current byte in the file

    //make sure we actually loaded data
    if (levelChars.size() == 0)
    {
        if(debugMode){std::cout << "Loaded empty file, data will not be processed!" << std::endl;}
        Level(false); //call blank constructor if empty file is loaded
    }
    else
    {
        //validate preamble exists
        if(levelChars.size() < 7)
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }
        //first four bytes in the file are the format version, stored as an int
        if(debugMode){std::cout << "Getting file format version..." << std::endl;}
        this->formatVer = readIntFromJava(levelChars, currentByte);
        currentByte += 4;
        
        if(debugMode){std::cout << "Format version " << this->formatVer << std::endl;}

        //the next byte is a bool seeing if custom graphics are enabled
        if(debugMode){std::cout << "Checking if custom graphics are enabled..." << std::endl;}
        this->customGraphicsEnabled = static_cast<bool>(levelChars.at(currentByte));
        if(this->customGraphicsEnabled)
        {
            if(debugMode){std::cout << "Custom graphics enabled, this feature is currently being studied and implemented. Expect strange behavior!" << std::endl;}
        }
        currentByte += 1;

        //the next two bytes are the number of blocks in the level, stored as a short
        if(debugMode){std::cout << "Reading block count..." << std::endl;}
        this->numBlockObjects = readShortFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numBlockObjects << " blocks in the level" << std::endl;}
        currentByte += 2;

        //One block uses nine bytes of data (bool + 2 ints = 1 + 2(4) = 9 bytes)
        //the next (9 * numBlocks) bytes are the data for each block

        if(levelChars.size() < (currentByte + (9 * numBlockObjects)))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        BlockObject *tempBlockObject = new BlockObject;
    
        for(int i = 0; i < this->numBlockObjects; i++)
        {

            tempBlockObject->objType = static_cast<int>(levelChars.at(currentByte));
            if(debugMode){std::cout << "The current block type is " << tempBlockObject->objType << std::endl;}
            currentByte++;
    
            tempBlockObject->xPos = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current block's xpos is " << tempBlockObject->xPos << std::endl;}
            currentByte += 4;
    
            tempBlockObject->yPos = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current block's ypos is " << tempBlockObject->yPos << std::endl;}
            currentByte += 4;
    
            tempBlockObject->indexInVec = i;
            if(debugMode){std::cout << "This block can be found at index " << tempBlockObject->indexInVec << std::endl;}
            this->blockObjects.push_back(*tempBlockObject);
    
            if(debugMode){std::cout << "Loaded object successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->blockObjects.size() << " object(s)!" << std::endl;}
        delete tempBlockObject;
        
        if(levelChars.size() < (currentByte + 8))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        //the next four bytes are the x position of the end of the level, stored as an int
        this->endPos = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "End wall is located at X position " << this->endPos << std::endl;}
        currentByte += 4;
    
        //the next four bytes are the number of color changes in the level, stored as an int
        if(debugMode){std::cout << "Attempting to read color change count" << std::endl;}
        this->numBackgroundChanges = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numBackgroundChanges << " color triggers in the level" << std::endl;}
        currentByte += 4;
    
        //Assuming all background changes don't use custom graphics
        //Each background change takes up 9 bytes (same math as before, 2 ints + 1 bool)
        //Therefore the next (9 * numBgSwitch) bytes are background changes
        if(levelChars.size() < (currentByte + (9 * this->numBackgroundChanges)))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        BackgroundChange *tempBackgroundChange = new BackgroundChange;
    
        for(int i = 0; i < this->numBackgroundChanges; i++)
        {
            tempBackgroundChange->xPos = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current color trigger's xpos is " << tempBackgroundChange->xPos << std::endl;};
            currentByte += 4;
    
            tempBackgroundChange->customTexture = static_cast<bool>(levelChars.at(currentByte));
            currentByte++;

            if(tempBackgroundChange->customTexture)
            {
                if(debugMode){std::cout << "Attempting to read custom texture" << std::endl;}

                tempBackgroundChange->filePath = readUTF8FromJava(levelChars, currentByte);

                 //The short int at currentByte represents how many characters are in the filePath
                currentByte += (readShortFromJava(levelChars, currentByte) + 2);

                if(debugMode){std::cout << "This backgroundchange requests the texture " << tempBackgroundChange->filePath << std::endl;}
                if(debugMode){std::cout << "Make sure it's defined in an atlas file!" << std::endl;}
                
                customTextures.push_back(tempBackgroundChange->filePath);
            }
            else
            {
                tempBackgroundChange->colorID = readIntFromJava(levelChars, currentByte);
                if(debugMode){std::cout << "The current color type is " << this->colorNames[tempBackgroundChange->colorID] << std::endl;}
                currentByte += 4;
            }
    
            tempBackgroundChange->indexInVec = i;
    
            if(debugMode){std::cout << "This color trigger can be found at index " << tempBackgroundChange->indexInVec << std::endl;}
            this->backgroundChanges.push_back(*tempBackgroundChange);
    
            if(debugMode){std::cout << "Loaded color trigger successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->backgroundChanges.size() << " color trigger(s)!" << std::endl;}
        delete tempBackgroundChange;
    
        //The next 4 bytes are the number of gravity changes in the level, stored as an int
        if(levelChars.size() < (currentByte + 4))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        if(debugMode){std::cout << "Attempting to read gravity change count" << std::endl;}
        this->numGravityChanges = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numGravityChanges << " gravity changes in the level" << std::endl;}
        currentByte += 4;
    
        //Each gravity change only takes up 4 bytes (1 int = 4 bytes)
        //Therefore, the next (4 * numGravitySwitch) bytes are gravity switch data
        if(levelChars.size() < (currentByte + (4 * this->numGravityChanges)))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        GravityChange *tempGravityChange = new GravityChange;
    
        for(int i = 0; i < this->numGravityChanges; i++)
        {
            tempGravityChange->xPos = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current gravity trigger's xpos is " << tempGravityChange->xPos << std::endl;}
            currentByte += 4;
    
            tempGravityChange->indexInVec = i;
    
            if(debugMode){std::cout << "This gravity trigger can be found at index " << tempGravityChange->indexInVec << std::endl;}
            this->gravityChanges.push_back(*tempGravityChange);
    
            if(debugMode){std::cout << "Loaded gravity trigger successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->gravityChanges.size() << " gravity trigger(s)!" << std::endl;}
        delete tempGravityChange;
    
        //The next 4 bytes are the number of falling block fade effects, stored as an int
        if(levelChars.size() < (currentByte + 4))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        if(debugMode){std::cout << "Attempting to read falling block count" << std::endl;}
        this->numBlocksFall = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numBlocksFall << " falling blocks in the level" << std::endl;}
        currentByte += 4;
    
        //Each falling block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numFallingBlocks) bytes are Falling Blocks data
        if(levelChars.size() < (currentByte + (8 * this->numBlocksFall)))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        BlocksFall *tempBlocksFall = new BlocksFall;
    
        for(int i = 0; i < this->numBlocksFall; i++)
        {
            tempBlocksFall->startX = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current falling block startX is " << tempBlocksFall->startX << std::endl;}
            currentByte += 4;
    
            tempBlocksFall->endX = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current falling block endX is " << tempBlocksFall->endX << std::endl;}
            currentByte += 4;
    
            tempBlocksFall->indexInVec = i;
    
            if(debugMode){std::cout << "This falling block section can be found at index " << tempBlocksFall->indexInVec << std::endl;}
            this->blocksFalls.push_back(*tempBlocksFall);
    
            if(debugMode){std::cout << "Loaded falling block section successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->blocksFalls.size() << " falling section(s)!" << std::endl;}
        delete tempBlocksFall;
    
        //The next 4 bytes are the number of rising block fade effects, stored as an int
        if(levelChars.size() < (currentByte + 4))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        if(debugMode){std::cout << "Attempting to read rising block count" << std::endl;}
        this->numBlocksRise = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numBlocksRise << " rising blocks in the level" << std::endl;}
        currentByte += 4;

        //Each rising block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numRisingBlocks) bytes are Rising Blocks data
        if(levelChars.size() < (currentByte + (8 * this->numBlocksRise)))
        {
            if(debugMode){std::cout << "ERROR: Reached EOF too soon" << std::endl;}
            loadedSuccessfully = false;
            return;
        }

        BlocksRise *tempBlocksRise = new BlocksRise;
    
        for(int i = 0; i < this->numBlocksRise; i++)
        {
            tempBlocksRise->startX = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current rising block startX is " << tempBlocksRise->startX << std::endl;}
            currentByte += 4;
    
            tempBlocksRise->endX = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current rising block endX is " << tempBlocksRise->endX << std::endl;}
            currentByte += 4;
    
            tempBlocksRise->indexInVec = i;
    
            if(debugMode){std::cout << "This rising block section can be found at index " << tempBlocksRise->indexInVec << std::endl;}
            this->blocksRises.push_back(*tempBlocksRise);
    
            if(debugMode){std::cout << "Loaded rising block section successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->blocksRises.size() << " rising section(s)!" << std::endl;}
        delete tempBlocksRise;
    }

    if(debugMode){std::cout << "Loaded entire level!" << std::endl;}
    loadedSuccessfully = true;
}

void Level::saveLevel(std::string filepath)
{
    if(!std::filesystem::exists(filepath))
    {
        std::filesystem::create_directory(filepath);
    }

    filepath += this->tig_filepath;

    this->mergeAdjacentPits(false);

    std::ofstream dataOut;
    dataOut.open(filepath.c_str(), std::ios_base::binary | std::ios_base::out);
    writeJavaInt(dataOut, this->formatVer);
    writeOtherData(dataOut, this->customGraphicsEnabled);

    writeJavaShort(dataOut, this->numBlockObjects);
    for(int i = 0; i < this->numBlockObjects; i++)
    {
        writeOtherData(dataOut, this->getBlockAtIndex(i)->objType);
        writeJavaInt(dataOut, this->getBlockAtIndex(i)->xPos);
        writeJavaInt(dataOut, this->getBlockAtIndex(i)->yPos);
    }
    writeJavaInt(dataOut, endPos);

    writeJavaInt(dataOut, numBackgroundChanges);
    for(int i = 0; i < numBackgroundChanges; i++)
    {
        writeJavaInt(dataOut, this->getBackgroundAtIndex(i)->xPos);
        writeOtherData(dataOut, this->getBackgroundAtIndex(i)->customTexture);
        if(this->getBackgroundAtIndex(i)->customTexture)
        {
            writeJavaUTF8(dataOut, this->getBackgroundAtIndex(i)->filePath);
        }
        else
        {
            writeJavaInt(dataOut, this->getBackgroundAtIndex(i)->colorID);
        }
    }

    writeJavaInt(dataOut, this->numGravityChanges);
    for(int i = 0; i < this->numGravityChanges; i++)
    {
        writeJavaInt(dataOut, this->getGravAtIndex(i)->xPos);
    }

    writeJavaInt(dataOut, this->numBlocksFall);
    for(int i = 0; i < this->numBlocksFall; i++)
    {
        writeJavaInt(dataOut, this->getFallingAtIndex(i)->startX);
        writeJavaInt(dataOut, this->getFallingAtIndex(i)->endX);
    }

    writeJavaInt(dataOut, this->numBlocksRise);
    for(int i = 0; i < this->numBlocksRise; i++)
    {
        writeJavaInt(dataOut, this->getRisingAtIndex(i)->startX);
        writeJavaInt(dataOut, this->getRisingAtIndex(i)->endX);
    }
}

//The following methods are all explained in the hpp file
int Level::getFormatVer()
{
    return this->formatVer;
}

BlockObject* Level::getBlockAtIndex(int index)
{
    if(index < this->numBlockObjects)
    {
        return &this->blockObjects[index];
    }
    
    return nullptr;
}

BackgroundChange* Level::getBackgroundAtIndex(int index)
{
    if(index < this->numBackgroundChanges)
    {
        return &this->backgroundChanges[index];
    }

    return nullptr;
}

GravityChange* Level::getGravAtIndex(int index)
{
    if(index < this->numGravityChanges)
    {
        return &this->gravityChanges[index];
    }

    return nullptr;
}

BlocksRise* Level::getRisingAtIndex(int index)
{
    if(index < this->numBlocksRise)
    {
        return &this->blocksRises[index];
    }

    return nullptr;
}

BlocksFall* Level::getFallingAtIndex(int index)
{
    if(index < this->numBlocksFall)
    {
        return &this->blocksFalls[index];
    }

    return nullptr;
}

int Level::getEndPos()
{
    return this->endPos;
}

int Level::getBlockCount()
{
    return this->numBlockObjects;
}

int Level::getBackgroundCount()
{
    return this->numBackgroundChanges;
}

int Level::getGravityCount()
{
    return this->numGravityChanges;
}

int Level::getRisingCount()
{
    return this->numBlocksRise;
}

int Level::getFallingCount()
{
    return this->numBlocksFall;
}

bool Level::getLoadedSuccessfully()
{
    return this->loadedSuccessfully;
}

void Level::addBlock(BlockObject *toAdd)
{
    toAdd->indexInVec = this->numBlockObjects;
    numBlockObjects++;
    this->blockObjects.push_back(*toAdd);
}

void Level::addBackground(BackgroundChange *toAdd)
{
    toAdd->indexInVec = this->numBackgroundChanges;
    numBackgroundChanges++;
    toAdd->colorName = this->colorNames[toAdd->colorID];
    this->backgroundChanges.push_back(*toAdd);
    if(toAdd->customTexture)
    {
        this->customGraphicsEnabled = true;
    }
}

void Level::addGravity(GravityChange *toAdd)
{
    toAdd->indexInVec = this->numGravityChanges;
    numGravityChanges++;
    this->gravityChanges.push_back(*toAdd);
}

void Level::addRising(BlocksRise *toAdd)
{
    toAdd->indexInVec = this->numBlocksRise;
    numBlocksRise++;
    this->blocksRises.push_back(*toAdd);
}

void Level::addFalling(BlocksFall *toAdd)
{
    toAdd->indexInVec = this->numBlocksFall;
    numBlocksFall++;
    this->blocksFalls.push_back(*toAdd);
}

void Level::setEndPos(int endPos)
{
    this->endPos = endPos;
}

//The if-else statements in the AtIndex methods make sure that the index is in range
void Level::removeBlockAtIndex(int index)
{
    if(index < this->numBlockObjects)
    {
        this->blockObjects.erase(this->blockObjects.begin() + index);
        this->numBlockObjects--;
    }
}

//The if-else statements in the removeLast methods make sure that the appropriate vectors contain data 
void Level::removeLastBlock()
{
    if(this->numBlockObjects > 0)
    {
        this->blockObjects.pop_back();
        this->numBlockObjects--;
    }
}

void Level::removeBackgroundAtIndex(int index)
{
    if(index < this->numBackgroundChanges)
    {
        this->backgroundChanges.erase(this->backgroundChanges.begin() + index);
        this->numBackgroundChanges--;
    }
}

void Level::removeLastBackground()
{
    if(this->numBackgroundChanges > 0)
    {
        this->backgroundChanges.pop_back();
        this->numBackgroundChanges--;
    }
}

void Level::removeGravityAtIndex(int index)
{
    if(index < this->numGravityChanges)
    {
        this->gravityChanges.erase(this->gravityChanges.begin() + index);
        this->numGravityChanges--;
    }
}

void Level::removeLastGravity()
{
    if(this->numGravityChanges > 0)
    {
        this->gravityChanges.pop_back();
        this->numGravityChanges--;
    }
}

void Level::removeRisingAtIndex(int index)
{
    if(index < this->numBlocksRise)
    {
        this->blocksRises.erase(this->blocksRises.begin() + index);
        this->numBlocksRise--;
    }
}

void Level::removeLastRising()
{
    if(this->numBlocksRise > 0)
    {
        this->blocksRises.pop_back();
        this->numBlocksRise--;
    }
}

void Level::removeFallingAtIndex(int index)
{
    if(index < this->numBlocksFall)
    {
        this->blocksFalls.erase(this->blocksFalls.begin() + index);
        this->numBlocksFall--;
    }
}

void Level::removeLastFalling()
{
    if(this->numBlocksFall > 0)
    {
        this->blocksFalls.pop_back();
        this->numBlocksFall--;
    }
}

void Level::mergeAdjacentPits(bool hitLastTime)
{
    sortBlocks();

    bool hitThisTime = false;

    if(hitLastTime == true)
    {
        for(int i = 0; i < numBlockObjects; i++)
        {
            if(blockObjects[i].objType == 2)
            {
                if(true){std::cout << "found pit at xpos " << blockObjects[i].xPos << std::endl;}
                for(int j = i+1; j < numBlockObjects; j++)
                {
                    if(blockObjects[j].objType == 2 && ((blockObjects[j].xPos == blockObjects[i].yPos) || (blockObjects[j].xPos < blockObjects[i].yPos)))
                    {
                        if(true){std::cout << "found adj pit at xpos " << blockObjects[j].xPos << std::endl;}
                        blockObjects[i].yPos = blockObjects[j].yPos;
                        removeBlockAtIndex(j);
                        hitThisTime = true;
                    }
                }
            }
        }

        mergeAdjacentPits(hitThisTime);
    }

}

void Level::sortBlocks()
{
    std::sort(blockObjects.begin(), blockObjects.end(), compareByPos);
}

void Level::printSummary()
{
    std::cout << "======Level Summary======" << std::endl;
    std::cout << "The level uses file format version " << formatVer << std::endl;
    std::cout << "The level contains " << numBlockObjects << " blocks" << std::endl;
    if(numBlockObjects > 0)
    {
        std::cout << "The first object is at X position " << (this->blockObjects[0].xPos) << std::endl;
    }
    std::cout << "The level ends at X position " << endPos << std::endl;
    std::cout << "The level has " << numBackgroundChanges << " background changes" << std::endl;
    if(customGraphicsEnabled)
    {
        std::cout << "The level has custom graphics enabled!" << std::endl;
        if(customTextures.size() > 0)
        {
            std::cout << "The level uses the following custom textures: ";
            for(int i = 0; i < customTextures.size(); i++)
            {
                std::cout << customTextures[i] << ", ";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "The level has " << numGravityChanges << " gravity changes" << std::endl;
    std::cout << "The level has " << numBlocksRise << " rising block triggers" << std::endl;
    std::cout << "The level has " << numBlocksFall << " falling block triggers" << std::endl;
    std::cout << std::endl;
}

