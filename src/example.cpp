#include "libImpossibleLevel.hpp"

//This is an example that generates a blank level
int main()
{
    Level leveldata("test.lvl", true);
    
    leveldata.printSummary();

    leveldata.saveLevel("test2.lvl");

    return 0;
}