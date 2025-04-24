#include "libImpossibleLevel.hpp"

//This is an example that generates a blank level
int main()
{
    Level leveldata("test.lvl", true);
    
    BlockObject* test1 = new BlockObject;
    test1->xPos = 650;
    test1->yPos = 590;
    test1->objType = 02;

    BlockObject* test2 = new BlockObject;
    test2->xPos = 500;
    test2->yPos = 560;
    test2->objType = 02;

    leveldata.addBlock(test1);
    leveldata.addBlock(test2);

    leveldata.printSummary();

    leveldata.saveLevel("test2.lvl");

    Level level2("test2.lvl", true);
    level2.mergeAdjacentPits();

    level2.printSummary();

    level2.saveLevel("test3.lvl");

    return 0;
}