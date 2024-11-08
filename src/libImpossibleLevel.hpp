#ifndef IMPOSSIBLE_LEVEL_LOADER
#define IMPOSSIBLE_LEVEL_LOADER

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

/*
BlockObj: short for "block object"
xPos = the object's x position in raw coordinates (divide by 30 to get its grid space)
yPos = the object's y position in raw coordinates
objType = the object's type ID (00 for a block, 01 for a spike, 02 for a pit)
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct BlockObj
{
    int xPos;
    int yPos;
    int objType;
    int indexInVec;
};

/*
BgCon: short for "background controller", i.e. the invisible objects that change the color or texture of the background
xPos = the object's x position in raw coordinates (divide by 30 to get its grid space)
colorId = the ID of the background color that will be used when this object is passed ()
customGraphics = the editor's seemingly-unused ability to load an actual texture image instead of using a color
customFile = the also unused UTF-8 string pointing to a texture
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct BgCon
{
    int xPos;
    int colorID;
    const char* colorName;
    bool customGraphics;
    const char* customFile;
    int indexInVec;
};

/*
GravityChange: the object that flips the level upside-down (only used in Chaoz Fantasy, impossible to place in the editor)
xPos = the object's x position in raw coordinates (divide by 30 to get its grid space)
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct GravityChange
{
    int xPos;
    int indexInVec;
};

/*
RisingBlocks: the object that enables the fade effect where blocks will rise from the ground on the right side of the screen
startX = the x positon where the effect will be enabled
endX = the x position where the effect will be disabled
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
Note: this and fallingBlocks could be merged into a single struct, but I don't want to merge them to keep true to the original code
*/
struct RisingBlocks
{
    int startX;
    int endX;
    int indexInVec;
};

/*
FallingBlocks: the object that enables the fade effect where blocks will fall to the ground on the left side of the screen
startX = the x positon where the effect will be enabled
endX = the x position where the effect will be disabled
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct FallingBlocks
{
    int startX;
    int endX;
    int indexInVec;
};

/*
The class that holds the actual level data

PUBLIC MEMBERS
- Level(): failsafe for if no parameters are given
- Level(char const*): call loadDataFromFile and pass its parameter
- ~Level(): deconstructor
- loadDataFromFile(char const*): load level data from path given by the parameter
- writeDataToFile(char const*): write data to path given by parameter
- getFormatVer(): get the file format version, returns an int
- getBlockAtIndex(int): returns the BlockObj at the given int if it exists, returns an empty BlockObj otherwise
- getBgConAtIndex(int): returns the BgCon at the given int if it exists, returns an empty BgCon otherwise
- getGravityAtIndex(int): returns the GravityChange at the given int if it exists, returns an empty GravityChange otherwise
- getRisingAtIndex(int): returns the RisingBlocks at the given int if it exists, returns an empty RisingBlocks otherwise
- getFallingAtIndex(int): returns the FallingBlocks at the given int if it exists, returns an empty FallingBlocks otherwise

*/
class Level
{
    public:
        Level();
        Level(char const*);
        ~Level();
        void loadDataFromFile(char const*);
        void writeDataToFile(char const*);

        //get methods
        int getFormatVer();
        BlockObj getBlockAtIndex(int);
        BgCon getBgConAtIndex(int);
        GravityChange getGravityAtIndex(int);
        RisingBlocks getRisingAtIndex(int);
        FallingBlocks getFallingAtIndex(int);
        int getEndPos();
        int getObjCount();
        int getBgCount();
        int getGravityCount();
        int getRisingCount();
        int getFallingCount();

        //set methods
        void addNewBlock(BlockObj);
        void addBgCon(BgCon);
        void addGravitySwitch(GravityChange);
        void addRisingBlocks(RisingBlocks);
        void addFallingBlocks(FallingBlocks);
        void setEndPos(int);

        //removal methods
        void removeBlockAtIndex(int);
        void removeLastBlock();
        void removeBgConAtIndex(int);
        void removeLastBgCon();
        void removeGravitySwitchAtIndex(int);
        void removeLastGravitySwitch();
        void removeRisingBlocksAtIndex(int);
        void removeLastRisingBlocks();
        void removeFallingBlocksAtIndex(int);
        void removeLastFallingBlocks();

        void printAllInfo();

    private:
        std::vector<BlockObj> *blockObjs = new std::vector<BlockObj>;
        std::vector<BgCon> *backgroundSwitches = new std::vector<BgCon>;
        std::vector<GravityChange> *gravitySwitches = new std::vector<GravityChange>;
        std::vector<RisingBlocks> *risingSections = new std::vector<RisingBlocks>;
        std::vector<FallingBlocks> *fallingSections = new std::vector<FallingBlocks>;
        short numBlocks;
        int numBgSwitch;
        int numGravitySwitch;
        int numRisingBlocks;
        int numFallingBlocks;
        int endWallPos;
        bool customGraphicsEnabled;
        int formatVer;
        const char* objNames[3] = {"Platform", "Spike", "Pit"}; 
        const char* colorNames[6] = {"blue", "yellow", "green", "violet", "pink", "black"};
};

#endif