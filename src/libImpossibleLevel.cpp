#include "libImpossibleLevel.hpp"

//Source: https://codereview.stackexchange.com/a/22907
//modified to convert to and then return an unsigned char vector on the heap instead of a signed one on the stack
static std::vector<unsigned char>* ReadAllBytes(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    if (pos == 0) {
        return new std::vector<unsigned char>;
    }

    std::vector<char>* result = new std::vector<char>(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(&result->at(0), pos);

    std::vector<unsigned char>* returnVal = new std::vector<unsigned char>(pos);
    for(int i = 0; i < result->size(); i++)
    {
        returnVal->at(i) = static_cast<unsigned char>(result->at(i));
    }

    delete result;
    return returnVal;
}

//Java handles evereything in big-Endian
//Since TIG's level editor is written in java, ints and shorts are written as big-Endian
//They must be converted to little-Endian after being read to be useable here
//This function takes an array of chars and a byte to start from. 
//It bit-shifts the starting byte and the next three bytes, then joins them together into a single int
//file = loaded file as a array of chars, startingOffset = the byte to start processing from
int readIntFromJava(std::vector<unsigned char> file, int startingOffset)
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
short readShortFromJava(std::vector<unsigned char> file, int startingOffset)
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

//Constructor that throws an error if no filepath is given
Level::Level()
{
    std::cout << "ERROR: Must provide a filepath!";
}

//Constructor that calls loadDataFromFile
Level::Level(char const* filename)
{
    this->loadDataFromFile(filename);
}

//Destructor that removes all data from the heap
Level::~Level()
{
    delete this->blockObjs;
    delete this->backgroundSwitches;
    delete this->gravitySwitches;
    delete this->risingSections;
    delete this->fallingSections;
}

//Parse level data from the given filepath, called by constructor
void Level::loadDataFromFile(char const* filepath)
{
    int currentByte = 0; //tracks the current byte in the file 
    std::vector<unsigned char> *level = ReadAllBytes(filepath); //load file from path, store in the heap
    
    //make sure we actually loaded data
    if (level->size() == 0)
    {
        std::cout << "Loaded empty file, data will not be processed!" << std::endl;
    }
    else
    {
        //first four bytes in the file are the format version, stored as an int
        std::cout << "getting file format version" << std::endl;
        this->formatVer = readIntFromJava(*level, currentByte);
        currentByte += 4;
        std::cout << "Format version " << this->formatVer << std::endl;

        //the next byte is a bool seeing if custom graphics are enabled
        std::cout << "Checking if special graphics are enabled" << std::endl;
        this->customGraphicsEnabled = static_cast<bool>(level->at(currentByte));
        if(this->customGraphicsEnabled)
        {
            std::cout << "Custom graphics enabled, this feature is undocumented, so the loader will likely break now" << std::endl;
        }
        currentByte += 1;

        //the next two bytes are the number of blocks in the level, stored as a short
        std::cout << "Attempting to read block count" << std::endl;
        this->numBlocks = readShortFromJava(*level, currentByte);
        std::cout << "There are " << this->numBlocks << " blocks in the level" << std::endl;
        currentByte += 2;

        //One block uses nine bytes of data (bool + 2 ints = 1 + 2(4) = 9 bytes)
        //the next (9 * numBlocks) bytes are the data for each block
        BlockObj *currentBlock = new BlockObj;
    
        for(int i = 0; i < this->numBlocks; i++)
        {

            currentBlock->objType = static_cast<int>(level->at(currentByte));
            std::cout << "The current block type is " << this->objNames[currentBlock->objType] << std::endl;
            currentByte++;
    
            currentBlock->xPos = readIntFromJava(*level, currentByte);
            std::cout << "The current block's xpos is " << currentBlock->xPos << std::endl;
            currentByte += 4;
    
            currentBlock->yPos = readIntFromJava(*level, currentByte);
            std::cout << "The current block's ypos is " << currentBlock->yPos << std::endl;
            currentByte += 4;
    
            currentBlock->indexInVec = i;
            std::cout << "This block can be found at index " << currentBlock->indexInVec << std::endl;
            this->blockObjs->push_back(*currentBlock);
    
            std::cout << "Loaded object successfully!" << std::endl;
        }
    
        std::cout << "Loaded " << this->blockObjs->size() << " object(s)!" << std::endl;
        delete currentBlock;
    
        //the next four bytes are the x position of the end of the level, stored as an int
        this->endWallPos = readIntFromJava(*level, currentByte);
        std::cout << "End wall is located at X position " << this->endWallPos << std::endl;
        currentByte += 4;
    
        //the next four bytes are the number of color changes in the level, stored as an int
        std::cout << "Attempting to read color change count" << std::endl;
        this->numBgSwitch = readIntFromJava(*level, currentByte);
        std::cout << "There are " << this->numBgSwitch << " color triggers in the level" << std::endl;
        currentByte += 4;
    
        //Assuming all background changes don't use custom graphics
        //Each background change takes up 9 bytes (same math as before, 2 ints + 1 bool)
        //Therefore the next (9 * numBgSwitch) bytes are background changes
        BgCon *currentBg = new BgCon;
    
        for(int i = 0; i < this->numBgSwitch; i++)
        {
            currentBg->xPos = readIntFromJava(*level, currentByte);
            std::cout << "The current color trigger's xpos is " << currentBg->xPos << std::endl;
            currentByte += 4;
    
            currentBg->customGraphics = static_cast<bool>(level->at(currentByte));
            currentByte++; //WILL BREAK HERE IF CUSTOM GRAPHICS ARE ENABLED FOR SOME REASON
    
            currentBg->colorID = readIntFromJava(*level, currentByte);
            std::cout << "The current color type is " << this->colorNames[currentBg->colorID] << std::endl;
            currentByte += 4;
    
            currentBg->indexInVec = i;
    
            std::cout << "This color trigger can be found at index " << currentBg->indexInVec << std::endl;
            this->backgroundSwitches->push_back(*currentBg);
    
            std::cout << "Loaded color trigger successfully!" << std::endl;
        }
    
        std::cout << "Loaded " << this->backgroundSwitches->size() << " color trigger(s)!" << std::endl;
        delete currentBg;
    
        //The next 4 bytes are the number of gravity changes in the level, stored as an int
        std::cout << "Attempting to read gravity change count" << std::endl;
        this->numGravitySwitch = readIntFromJava(*level, currentByte);
        std::cout << "There are " << this->numGravitySwitch << " gravity changes in the level" << std::endl;
        currentByte += 4;
    
        //Each gravity change only takes up 4 bytes (1 int = 4 bytes)
        //Therefore, the next (4 * numGravitySwitch) bytes are gravity switch data
        GravityChange *currentGrav = new GravityChange;
    
        for(int i = 0; i < this->numGravitySwitch; i++)
        {
            currentGrav->xPos = readIntFromJava(*level, currentByte);
            std::cout << "The current gravity trigger's xpos is " << currentGrav->xPos << std::endl;
            currentByte += 4;
    
            currentGrav->indexInVec = i;
    
            std::cout << "This gravity trigger can be found at index " << currentGrav->indexInVec << std::endl;
            this->gravitySwitches->push_back(*currentGrav);
    
            std::cout << "Loaded gravity trigger successfully!" << std::endl;
        }
    
        std::cout << "Loaded " << this->gravitySwitches->size() << " gravity trigger(s)!" << std::endl;
        delete currentGrav;
    
        //The next 4 bytes are the number of falling block fade effects, stored as an int
        std::cout << "Attempting to read falling block count" << std::endl;
        this->numFallingBlocks = readIntFromJava(*level, currentByte);
        std::cout << "There are " << this->numFallingBlocks << " falling blocks in the level" << std::endl;
        currentByte += 4;
    
        //Each falling block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numFallingBlocks) bytes are Falling Blocks data
        FallingBlocks *currentFalling = new FallingBlocks;
    
        for(int i = 0; i < this->numFallingBlocks; i++)
        {
            currentFalling->startX = readIntFromJava(*level, currentByte);
            std::cout << "The current falling block startX is " << currentFalling->startX << std::endl;
            currentByte += 4;
    
            currentFalling->endX = readIntFromJava(*level, currentByte);
            std::cout << "The current falling block endX is " << currentFalling->endX << std::endl;
            currentByte += 4;
    
            currentFalling->indexInVec = i;
    
            std::cout << "This falling block section can be found at index " << currentFalling->indexInVec << std::endl;
            this->fallingSections->push_back(*currentFalling);
    
            std::cout << "Loaded falling block section successfully!" << std::endl;
        }
    
        std::cout << "Loaded " << this->fallingSections->size() << " falling section(s)!" << std::endl;
        delete currentFalling;
    
        //The next 4 bytes are the number of rising block fade effects, stored as an int
        std::cout << "Attempting to read rising block count" << std::endl;
        this->numRisingBlocks = readIntFromJava(*level, currentByte);
        std::cout << "There are " << numRisingBlocks << " rising blocks in the level" << std::endl;
        currentByte += 4;

        //Each rising block object takes up 8 bytes (2 ints = 2 * 4 bytes = 8 bytes)
        //Therefore the next (8 * numRisingBlocks) bytes are Rising Blocks data
        RisingBlocks *currentRising = new RisingBlocks;
    
        for(int i = 0; i < this->numFallingBlocks; i++)
        {
            currentRising->startX = readIntFromJava(*level, currentByte);
            std::cout << "The current rising block startX is " << currentRising->startX << std::endl;
            currentByte += 4;
    
            currentRising->endX = readIntFromJava(*level, currentByte);
            std::cout << "The current rising block endX is " << currentRising->endX << std::endl;
            currentByte += 4;
    
            currentRising->indexInVec = i;
    
            std::cout << "This rising block section can be found at index " << currentRising->indexInVec << std::endl;
            this->risingSections->push_back(*currentRising);
    
            std::cout << "Loaded rising block section successfully!" << std::endl;
        }
    
        std::cout << "Loaded " << this->risingSections->size() << " rising section(s)!" << std::endl;
        delete currentRising;
    }

    std::cout << "Loaded entire level!" << std::endl;
    delete level; //all data is now stored in the class, the raw level data is no longer needed
}

void Level::writeDataToFile(char const* filepath)
{
    std::ofstream dataOut;
    dataOut.open(filepath, std::ios_base::binary | std::ios_base::out);
    writeJavaInt(dataOut, this->formatVer);
    writeOtherData(dataOut, this->customGraphicsEnabled);
    writeJavaShort(dataOut, this->numBlocks);
    BlockObj temp;
    for(int i = 0; i < this->numBlocks; i++)
    {
        temp = this->getBlockAtIndex(i);
        writeOtherData(dataOut, temp.objType);
        writeJavaInt(dataOut, temp.xPos);
        writeJavaInt(dataOut, temp.yPos);
    }
    writeJavaInt(dataOut, endWallPos);
    writeJavaInt(dataOut, numBgSwitch);
    BgCon tempCon;
    for(int i = 0; i < numBgSwitch; i++)
    {
        tempCon = this->getBgConAtIndex(i);
        writeJavaInt(dataOut, tempCon.xPos);
        writeOtherData(dataOut, tempCon.customGraphics);
        //Will need reworking once I can figure out how the custom graphics work
        if(tempCon.customGraphics)
        {
            std::cout << "the program will break now" << std::endl;
        }
        else
        {
            writeJavaInt(dataOut, tempCon.colorID);
        }
    }
    writeJavaInt(dataOut, this->numGravitySwitch);
    GravityChange tempGrav;
    for(int i = 0; i < this->numGravitySwitch; i++)
    {
        tempGrav = this->getGravityAtIndex(i);
        writeJavaInt(dataOut, tempGrav.xPos);
    }
    writeJavaInt(dataOut, this->numRisingBlocks);
    RisingBlocks tempRising;
    for(int i = 0; i < this->numRisingBlocks; i++)
    {
        tempRising = this->getRisingAtIndex(i);
        writeJavaInt(dataOut, tempRising.startX);
        writeJavaInt(dataOut, tempRising.endX);
    }
    writeJavaInt(dataOut, this->numFallingBlocks);
    FallingBlocks tempFalling;
    for(int i = 0; i < this->numFallingBlocks; i++)
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

BlockObj Level::getBlockAtIndex(int index)
{
    if(index < this->numBlocks)
    {
        return this->blockObjs->at(index);
    }
    else
    {
        BlockObj nullObj = {0, 0, 0, 0};
        return nullObj;
    }
}

BgCon Level::getBgConAtIndex(int index)
{
    if(index < this->numBgSwitch)
    {
        return this->backgroundSwitches->at(index);
    }
    else
    {
        BgCon nullObj = {0, 0, "null", false, "null", 0};
        return nullObj;
    }

}

GravityChange Level::getGravityAtIndex(int index)
{
    if(index < this->numGravitySwitch)
    {
        return this->gravitySwitches->at(index);
    }
    else
    {
        GravityChange nullObj = {0, 0};
        return nullObj;
    }
}

RisingBlocks Level::getRisingAtIndex(int index)
{
    if(index < this->numRisingBlocks)
    {
        return this->risingSections->at(index);
    }
    else
    {
        RisingBlocks nullObj = {0, 0, 0};
        return nullObj;
    }
}

FallingBlocks Level::getFallingAtIndex(int index)
{
    if(index < this->numFallingBlocks)
    {
        return this->fallingSections->at(index);
    }
    else
    {
        FallingBlocks nullObj = {0, 0, 0};
        return nullObj;
    }
}

int Level::getEndPos()
{
    return this->endWallPos;
}

int Level::getObjCount()
{
    return this->numBlocks;
}

int Level::getBgCount()
{
    return this->numBgSwitch;
}

int Level::getGravityCount()
{
    return this->numGravitySwitch;
}

int Level::getRisingCount()
{
    return this->numRisingBlocks;
}

int Level::getFallingCount()
{
    return this->numFallingBlocks;
}

void Level::addNewBlock(BlockObj toAdd)
{
    toAdd.indexInVec = this->numBlocks;
    numBlocks++;
    this->blockObjs->push_back(toAdd);
}

void Level::addBgCon(BgCon toAdd)
{
    toAdd.indexInVec = this->numBgSwitch;
    numBgSwitch++;
    toAdd.colorName = this->colorNames[toAdd.colorID];
    this->backgroundSwitches->push_back(toAdd);
}

void Level::addGravitySwitch(GravityChange toAdd)
{
    toAdd.indexInVec = this->numGravitySwitch;
    numGravitySwitch++;
    this->gravitySwitches->push_back(toAdd);
}

void Level::addRisingBlocks(RisingBlocks toAdd)
{
    toAdd.indexInVec = this->numRisingBlocks;
    numRisingBlocks++;
    this->risingSections->push_back(toAdd);
}

void Level::addFallingBlocks(FallingBlocks toAdd)
{
    toAdd.indexInVec = this->numFallingBlocks;
    numFallingBlocks++;
    this->fallingSections->push_back(toAdd);
}

void Level::setEndPos(int endPos)
{
    this->endWallPos = endPos;
}

//The if-else statements in the AtIndex methods make sure that the index is in range
void Level::removeBlockAtIndex(int index)
{
    if(index < this->numBlocks)
    {
        this->blockObjs->erase(this->blockObjs->begin() + index);
        this->numBlocks--;
    }
}

//The if-else statements in the removeLast methods make sure that the appropriate vectors contain data 
void Level::removeLastBlock()
{
    if(this->numBlocks > 0)
    {
        this->blockObjs->pop_back();
        this->numBlocks--;
    }
}

void Level::removeBgConAtIndex(int index)
{
    if(index < this->numBgSwitch)
    {
        this->backgroundSwitches->erase(this->backgroundSwitches->begin() + index);
        this->numBgSwitch--;
    }
}

void Level::removeLastBgCon()
{
    if(this->numBgSwitch > 0)
    {
        this->backgroundSwitches->pop_back();
        this->numBgSwitch--;
    }
}

void Level::removeGravitySwitchAtIndex(int index)
{
    if(index < this->numGravitySwitch)
    {
        this->gravitySwitches->erase(this->gravitySwitches->begin() + index);
        this->numGravitySwitch--;
    }
}

void Level::removeLastGravitySwitch()
{
    if(this->numGravitySwitch > 0)
    {
        this->gravitySwitches->pop_back();
        this->numGravitySwitch--;
    }
}

void Level::removeRisingBlocksAtIndex(int index)
{
    if(index < this->numRisingBlocks)
    {
        this->risingSections->erase(this->risingSections->begin() + index);
        this->numRisingBlocks--;
    }
}

void Level::removeLastRisingBlocks()
{
    if(this->numRisingBlocks > 0)
    {
        this->risingSections->pop_back();
        this->numRisingBlocks--;
    }
}

void Level::removeFallingBlocksAtIndex(int index)
{
    if(index < this->numFallingBlocks)
    {
        this->fallingSections->erase(this->fallingSections->begin() + index);
        this->numFallingBlocks--;
    }
}

void Level::removeLastFallingBlocks()
{
    if(this->numFallingBlocks > 0)
    {
        this->fallingSections->pop_back();
        this->numFallingBlocks--;
    }
}

void Level::printAllInfo()
{

}
