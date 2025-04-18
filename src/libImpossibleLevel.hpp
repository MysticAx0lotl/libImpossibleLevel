#ifndef IMPOSSIBLE_LEVEL_LOADER
#define IMPOSSIBLE_LEVEL_LOADER

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>

/*
BlockObj: short for "block object"
xPos = the object's x position in raw coordinates (divide by 30 to get its grid space)
yPos = the object's y position in raw coordinates (instead serves as the ending x position for pits)
objType = the object's type ID (00 for a block, 01 for a spike, 02 for a pit)
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct BlockObject
{
    int xPos;
    int yPos;
    int objType;
    int indexInVec;
};

/*
BackgroundChange: the invisible objects that change the color or texture of the background
xPos = the object's x position in raw coordinates (divide by 30 to get its grid space)
colorId = the ID of the background color that will be used when this object is passed ()
customGraphics = the editor's seemingly-unused ability to load an actual texture image instead of using a color
customFile = the also unused UTF-8 string pointing to a texture
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct BackgroundChange
{
    int xPos;
    int colorID;
    const char* colorName;
    bool customTexture;
    std::string filePath;
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
BlocksRise: the object that enables the fade effect where blocks will rise from the ground on the right side of the screen
startX = the x positon where the effect will be enabled
endX = the x position where the effect will be disabled
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
Note: this and fallingBlocks could be merged into a single struct, but I don't want to merge them to keep true to the original code
*/
struct BlocksRise
{
    int startX;
    int endX;
    int indexInVec;
};

/*
BlocksFall: the object that enables the fade effect where blocks will fall to the ground on the left side of the screen
startX = the x positon where the effect will be enabled
endX = the x position where the effect will be disabled
indexInVec = the object's index in the vector it's part of. Only used by this library, does not get read from or written to the level file
*/
struct BlocksFall
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
- getEndPos(): return the x position of the end wall as an int
- getObjCount() through getFallingCount(): return the size of the corresponding vectors
- addNewBlock(BlockObj) through addFallingBlocks(FallingBlocks): add the provided object to the corresponding vector, and increment the associated private member
- setEndPos(int): set the position of the end wall to the given parameter
- remove____AtIndex(int): removes the object at the given index from the appropriate vector, if the given index is in bounds
- removeLast____(): removes the last entry from the appropriate vector, if the appropriate vector isn't empty
- printSummary(): prints a short summary including the format version, which objects the level has and how many, which custom textures the level uses if any, and where the level starts and ends

PRIVATE MEMBERS
- blockObjs: pointer to a vector of BlockObj objects, stored in the heap
- backgroundSwitches: pointer to a vector of BgCon objects, stored in the heap
- gravitySwitches: pointer to a vector of GravityChange objects, stored in the heap
- risingSections: pointer to a vector of RisingBlocks objects, stored in the heap
- fallingSections: pointer to a vector of FallingBlocks objects, stored in the heap
- numBlocks: size of blockObjs, stored as a short
- numBgSwitch through numFallingBlocks: size of the associated vector, stored as an int
- endWallPos: x position of the end of the level, stored as an int
- customGraphicsEnabled: whether or not custom graphics are enabled for the level, stored as a bool
- formatVer: the file format version, stored as an int
- objNames: the human-readable name corresponding to each object ID, stored as an array of const chars
- colorNames: the human-readable name corresponding to each color ID, stored as an array of const chars
*/
class Level
{
    public:
        Level(bool);
        Level(std::string, bool);
        Level(std::vector<unsigned char>, bool);
        ~Level();
        void loadLevel(std::vector<unsigned char>, bool);
        void saveLevel(std::string);

        //get methods
        int getFormatVer();
        BlockObject* getBlockAtIndex(int);
        BackgroundChange* getBackgroundAtIndex(int);
        GravityChange* getGravAtIndex(int);
        BlocksRise* getRisingAtIndex(int);
        BlocksFall* getFallingAtIndex(int);
        int getEndPos();
        int getBlockCount();
        int getBackgroundCount();
        int getGravityCount();
        int getRisingCount();
        int getFallingCount();

        //set methods
        void addBlock(BlockObject*);
        void addBackground(BackgroundChange*);
        void addGravity(GravityChange*);
        void addRising(BlocksRise*);
        void addFalling(BlocksFall*);
        void setEndPos(int);

        //removal methods
        void removeBlockAtIndex(int);
        void removeLastBlock();
        void removeBackgroundAtIndex(int);
        void removeLastBackground();
        void removeGravityAtIndex(int);
        void removeLastGravity();
        void removeRisingAtIndex(int);
        void removeLastRising();
        void removeFallingAtIndex(int);
        void removeLastFalling();

        void printSummary();

    private:
        std::vector<BlockObject> blockObjects;
        std::vector<BackgroundChange> backgroundChanges;
        std::vector<GravityChange> gravityChanges;
        std::vector<BlocksRise> blocksRises;
        std::vector<BlocksFall> blocksFalls;
        std::vector<std::string> customTextures;
        short numBlockObjects;
        int numBackgroundChanges;
        int numGravityChanges;
        int numBlocksRise;
        int numBlocksFall;
        int endPos;
        bool customGraphicsEnabled;
        int formatVer;
        static inline const char* blockNames[3] = {"Platform", "Spike", "Pit"}; 
        static inline const char* colorNames[6] = {"blue", "yellow", "green", "violet", "pink", "black"};
        static inline const std::string tig_filepath = "/level.dat";
};

#endif
