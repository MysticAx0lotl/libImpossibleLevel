#include "libImpossibleLevel.hpp"

//This is an example that generates a blank level
int main()
{
    Level leveldata(false);
    
    leveldata.saveLevel("blank.lvl");

    return 0;
}