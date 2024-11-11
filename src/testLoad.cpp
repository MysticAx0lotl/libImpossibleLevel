#include "libImpossibleLevel.hpp"

//This is an example that parses a level file, and adds a spike object at grid space (31, 1)
int main()
{
    Level leveldata("demo.lvl/level.dat", true);
	leveldata.removeLastBackground();
    leveldata.saveLevel("output.dat");

    Level leveldata2("output.dat", true);
    BackgroundChange newBG;
    newBG.customTexture = true;
    newBG.filePath = "real.png";
    newBG.xPos = 870;
    leveldata2.addBackground(&newBG);
    leveldata2.saveLevel("customtest.dat");
    return 0;
}