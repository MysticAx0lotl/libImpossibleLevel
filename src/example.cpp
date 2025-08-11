#include "libImpossibleLevel.hpp"

//This is an example that generates a blank level
int main()
{
    Level leveldata(false);
    
    BlockObject* testPit = new BlockObject;
    *testPit = {30, 60, 02, 0};
    BlockObject* testPit2 = new BlockObject;
    *testPit2 = {40, 90, 02, 0};
    leveldata.addBlock(testPit);
    leveldata.addBlock(testPit2);

    BlockObject* testPit3 = new BlockObject;
    *testPit3 = {10, 30, 02, 0};
    BlockObject* testPit4 = new BlockObject;
    *testPit4 = {40, 120, 02, 0};
    leveldata.addBlock(testPit3);
    leveldata.addBlock(testPit4);

    leveldata.printSummary();

    leveldata.mergeAdjacentPits(false);

    leveldata.printSummary();

    std::cout << leveldata.getBlockAtIndex(0)->xPos << std::endl;
    std::cout << leveldata.getBlockAtIndex(0)->yPos << std::endl;

    return 0;
}