#ifndef IMPOSSIBLE_LEVEL_LOADER
#define IMPOSSIBLE_LEVEL_LOADER

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

//using namespace std;

struct blockObj
{
    int xPos;
    int yPos;
    int objType;
    int indexInVec; //used for vectors specific to this library
};

struct bgCon //background switcher
{
    int xPos;
    int colorID;
    const char* colorName;
    int indexInVec;
};

struct gravityChange
{
    int xPos;
    int indexInVec;
};

struct risingBlocks //this and fallingBlocks could be merged into a single struct
{
    int startX;
    int endX;
    int indexInVec;
};

struct fallingBlocks //but I don't want to merge them to keep true to the original code
{
    int startX;
    int endX;
    int indexInVec;
};

class levelObj
{
    public:
        levelObj();
        levelObj(char const*);
        void loadDataFromFile(char const*);
        void writeDataToFile(char const*);

        //get methods
        int getFormatVer();
        blockObj getBlockAtIndex(int);
        bgCon getBgConAtIndex(int);
        gravityChange getGravityAtIndex(int);
        risingBlocks getRisingAtIndex(int);
        fallingBlocks getFallingAtIndex(int);
        int getEndPos();
        int getObjCount();
        int getBgCount();
        int getGravityCount();
        int getRisingCount();
        int getFallingCount();

        //set methods
        void addNewBlock(blockObj);
        void addBgCon(bgCon);
        void addGravitySwitch(gravityChange);
        void addRisingBlocks(risingBlocks);
        void addFallingBlocks(fallingBlocks);
        void setEndPos(int);

        //removal methods


        void printAllInfo();

    private:
        std::vector<blockObj> *blockObjs = new std::vector<blockObj>;
        std::vector<bgCon> *backgroundSwitches = new std::vector<bgCon>;
        std::vector<gravityChange> *gravitySwitches = new std::vector<gravityChange>;
        std::vector<risingBlocks> *risingSections = new std::vector<risingBlocks>;
        std::vector<fallingBlocks> *fallingSections = new std::vector<fallingBlocks>;
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