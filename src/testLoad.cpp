#include "libImpossibleLevel.hpp"

//This is an example that parses a level file, and adds a spike object at grid space (31, 1)
int main()
{
    Level leveldata("demo.lvl/level.dat");
    BlockObj newBlock = {930, 30, 00, 0};
    leveldata.addNewBlock(newBlock);
    GravityChange gravity = {930, 0};
    leveldata.addGravitySwitch(gravity);
    leveldata.writeDataToFile("output.lvl/level.dat");
    return 0;
}