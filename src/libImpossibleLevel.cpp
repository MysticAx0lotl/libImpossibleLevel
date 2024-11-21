#include "libImpossibleLevel.hpp"

//Source: https://codereview.stackexchange.com/a/22907
//modified to convert to and then return an unsigned char vector instead of a signed one
static std::vector<unsigned char> ReadAllBytes(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    if (pos == 0) {
        return std::vector<unsigned char>{};
    }

    std::vector<char>  result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    std::vector<unsigned char> returnVal(pos);
    for(int i = 0; i < result.size(); i++)
    {
        returnVal[i] = static_cast<unsigned char>(result[i]);
    }

    return returnVal;
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

//Endianess doesn't matter for bools or char arrays, this function handles exporting those
void writeOtherData(std::ofstream& datafile, unsigned char data)
{
    datafile.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

//Constructor that generates a blank level if no filepath is given
Level::Level(bool debugMode)
{
    if(debugMode){std::cout << "No filepath was given, a blank level will be generated..." << std::endl;}

    numBlockObjects = 0;
    numBackgroundChanges = 0;
    numGravityChanges = 0;
    numBlocksRise = 0;
    numBlocksFall = 0;
    endPos = 3015;
    customGraphicsEnabled = false;
    formatVer = 0;

    if(debugMode){std::cout << "Blank level generated!" << std::endl;}
}

//Constructor that calls loadDataFromFile
Level::Level(std::string filename, bool debugMode)
{
    filename += "/level.dat";
    this->loadLevel(filename.c_str(), debugMode);
}

//Destructor that serves no purpose right now
Level::~Level()
{

}

//Parse level data from the given filepath, called by constructor
void Level::loadLevel(char const* filepath, bool debugMode)
{
    int currentByte = 0; //tracks the current byte in the file 
    if(debugMode){std::cout << "Attempting to read file " << filepath << std::endl;}
    std::vector<unsigned char> levelChars = ReadAllBytes(filepath); //load file from path, store in the heap
    
    //make sure we actually loaded data
    if (levelChars.size() == 0)
    {
        if(debugMode){std::cout << "Loaded empty file, data will not be processed!" << std::endl;}
        Level(false); //call blank constructor if empty file is loaded
    }
    else
    {
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
        BackgroundChange *tempBackgroundChage = new BackgroundChange;
    
        for(int i = 0; i < this->numBackgroundChanges; i++)
        {
            tempBackgroundChage->xPos = readIntFromJava(levelChars, currentByte);
            if(debugMode){std::cout << "The current color trigger's xpos is " << tempBackgroundChage->xPos << std::endl;};
            currentByte += 4;
    
            tempBackgroundChage->customTexture = static_cast<bool>(levelChars.at(currentByte));
            currentByte++; //WILL CURRENTLY BREAK HERE IF CUSTOM GRAPHICS ARE ENABLED

            if(tempBackgroundChage->customTexture)
            {
                if(debugMode){std::cout << "Custom background support is currently a stub" << std::endl;}

                //DO NOT FORGET TO ADD THE REQUESTED FILE TO customTextures!!!!!!!
            }
            else
            {
                tempBackgroundChage->colorID = readIntFromJava(levelChars, currentByte);
                if(debugMode){std::cout << "The current color type is " << this->colorNames[tempBackgroundChage->colorID] << std::endl;}
                currentByte += 4;
            }
    
            tempBackgroundChage->indexInVec = i;
    
            if(debugMode){std::cout << "This color trigger can be found at index " << tempBackgroundChage->indexInVec << std::endl;}
            this->backgroundChanges.push_back(*tempBackgroundChage);
    
            if(debugMode){std::cout << "Loaded color trigger successfully!" << std::endl;}
        }
    
        if(debugMode){std::cout << "Loaded " << this->backgroundChanges.size() << " color trigger(s)!" << std::endl;}
        delete tempBackgroundChage;
    
        //The next 4 bytes are the number of gravity changes in the level, stored as an int
        if(debugMode){std::cout << "Attempting to read gravity change count" << std::endl;}
        this->numGravityChanges = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numGravityChanges << " gravity changes in the level" << std::endl;}
        currentByte += 4;
    
        //Each gravity change only takes up 4 bytes (1 int = 4 bytes)
        //Therefore, the next (4 * numGravitySwitch) bytes are gravity switch data
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
        if(debugMode){std::cout << "Attempting to read falling block count" << std::endl;}
        this->numBlocksFall = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << this->numBlocksFall << " falling blocks in the level" << std::endl;}
        currentByte += 4;
    
        //Each falling block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numFallingBlocks) bytes are Falling Blocks data
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
        if(debugMode){std::cout << "Attempting to read rising block count" << std::endl;}
        this->numBlocksRise = readIntFromJava(levelChars, currentByte);
        if(debugMode){std::cout << "There are " << numBlocksRise << " rising blocks in the level" << std::endl;}
        currentByte += 4;

        //Each rising block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numRisingBlocks) bytes are Rising Blocks data
        BlocksRise *tempBlocksRise = new BlocksRise;
    
        for(int i = 0; i < this->numBlocksFall; i++)
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
}

void Level::saveLevel(std::string filepath)
{
    if(!std::filesystem::exists(filepath))
    {
        std::filesystem::create_directory(filepath);
    }

    filepath += "/level.dat";

    std::ofstream dataOut;
    dataOut.open(filepath.c_str(), std::ios_base::binary | std::ios_base::out);
    writeJavaInt(dataOut, this->formatVer);
    writeOtherData(dataOut, this->customGraphicsEnabled);
    writeJavaShort(dataOut, this->numBlockObjects);
    BlockObject temp;
    for(int i = 0; i < this->numBlockObjects; i++)
    {
        temp = this->getBlockAtIndex(i);
        writeOtherData(dataOut, temp.objType);
        writeJavaInt(dataOut, temp.xPos);
        writeJavaInt(dataOut, temp.yPos);
    }
    writeJavaInt(dataOut, endPos);
    writeJavaInt(dataOut, numBackgroundChanges);
    BackgroundChange tempCon;
    for(int i = 0; i < numBackgroundChanges; i++)
    {
        tempCon = this->getBackgroundAtIndex(i);
        writeJavaInt(dataOut, tempCon.xPos);
        writeOtherData(dataOut, tempCon.customTexture);
        if(tempCon.customTexture)
        {
            short strLen = sizeof(tempCon.filePath);
            writeJavaShort(dataOut, strLen);
            for(int i = 0; i < sizeof(tempCon.filePath); i++)
            {
                writeOtherData(dataOut, tempCon.filePath[i]);
            }
        }
        else
        {
            writeJavaInt(dataOut, tempCon.colorID);
        }
    }
    writeJavaInt(dataOut, this->numGravityChanges);
    GravityChange tempGrav;
    for(int i = 0; i < this->numGravityChanges; i++)
    {
        tempGrav = this->getGravAtIndex(i);
        writeJavaInt(dataOut, tempGrav.xPos);
    }
    writeJavaInt(dataOut, this->numBlocksRise);
    BlocksRise tempRising;
    for(int i = 0; i < this->numBlocksRise; i++)
    {
        tempRising = this->getRisingAtIndex(i);
        writeJavaInt(dataOut, tempRising.startX);
        writeJavaInt(dataOut, tempRising.endX);
    }
    writeJavaInt(dataOut, this->numBlocksFall);
    BlocksFall tempFalling;
    for(int i = 0; i < this->numBlocksFall; i++)
    {
        tempFalling = this->getFallingAtIndex(i);
        writeJavaInt(dataOut, tempFalling.startX);
        writeJavaInt(dataOut, tempFalling.startX);
    }
}

//The following methods are all explained in the hpp file
int Level::getFormatVer()
{
    return this->formatVer;
}

BlockObject& Level::getBlockAtIndex(int index)
{
    if(index < this->numBlockObjects)
    {
        return this->blockObjects.at(index);
    }
    else
    {
        static BlockObject nullObj = {0, 0, 0, 0};
        return nullObj;
    }
}

BackgroundChange& Level::getBackgroundAtIndex(int index)
{
    if(index < this->numBackgroundChanges)
    {
        return this->backgroundChanges.at(index);
    }
    else
    {
        static BackgroundChange nullObj = {0, 0, "null", false, "null", 0};
        return nullObj;
    }

}

GravityChange& Level::getGravAtIndex(int index)
{
    if(index < this->numGravityChanges)
    {
        return this->gravityChanges.at(index);
    }
    else
    {
        static GravityChange nullObj = {0, 0};
        return nullObj;
    }
}

BlocksRise& Level::getRisingAtIndex(int index)
{
    if(index < this->numBlocksRise)
    {
        return this->blocksRises.at(index);
    }
    else
    {
        static BlocksRise nullObj = {0, 0, 0};
        return nullObj;
    }
}

BlocksFall& Level::getFallingAtIndex(int index)
{
    if(index < this->numBlocksFall)
    {
        return this->blocksFalls.at(index);
    }
    else
    {
        static BlocksFall nullObj = {0, 0, 0};
        return nullObj;
    }
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

