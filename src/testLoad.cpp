#include "libImpossibleLevel.hpp"

//This is an example that parses a level file, and adds a spike object at grid space (31, 1)
int main()
{
    levelObj leveldata("demo.lvl/level.dat");
    blockObj newBlock = {930, 30, 01, 0};
    leveldata.addNewBlock(newBlock);
    leveldata.writeDataToFile("output.lvl/level.dat");
    return 0;
}