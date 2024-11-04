#include "libImpossibleLevel.hpp"

int main()
{
    levelObj leveldata("demo.lvl/level.dat");
    blockObj newBlock = {930, 30, 01, 0};
    leveldata.addNewBlock(newBlock);
    leveldata.writeDataToFile("output.lvl/level.dat");
    return 0;
}