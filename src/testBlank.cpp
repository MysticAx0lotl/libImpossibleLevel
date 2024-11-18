#include "libImpossibleLevel.hpp"

//This is an example that generates a blank level
int main()
{
    Level leveldata(false);
    leveldata.saveLevel("output.dat");

    return 0;
}