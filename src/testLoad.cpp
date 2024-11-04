#include "libImpossibleLevel.hpp"

int main()
{
    levelObj leveldata("demo.lvl/level.dat");
    leveldata.writeDataToFile("testOut.dat");
    return 0;
}