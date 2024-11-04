
static std::vector<unsigned char> ReadAllBytes(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    std::ifstream::pos_type pos = ifs.tellg();

    if (pos == 0) {
        return std::vector<unsigned char>{};
    }

    std::vector<char>  result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    std::vector<unsigned char> returnVal(pos);
    for(int i = 0; i < result.size(); i++)
    {
        returnVal[i] = static_cast<unsigned char>(result[i]);
    }

    return returnVal;
}

int readIntFromJava(vector<unsigned char> file, int startingOffset)
{
    unsigned int bit1, bit2, bit3, bit4;
    bit1 = static_cast<unsigned int>(file[startingOffset]);
    bit2 = static_cast<unsigned int>(file[startingOffset + 1]);
    bit3 = static_cast<unsigned int>(file[startingOffset + 2]);
    bit4 = static_cast<unsigned int>(file[startingOffset + 3]);

    bit1 = bit1 << 24;
    bit2 = bit2 << 16;
    bit3 = bit3 << 8;
    //bit 4 doesn't get shifted

    unsigned int resultU = bit1 | bit2 | bit3 | bit4;
    //unsigned int resultU = bit3 | bit4;
    int result = static_cast<int>(resultU);
    return result;
}

short readShortFromJava(vector<unsigned char> file, int startingOffset)
{
    unsigned short bit1, bit2;
    bit1 = static_cast<unsigned int>(file[startingOffset]);
    bit2 = static_cast<unsigned int>(file[startingOffset + 1]);

    bit1 = bit1 << 8;
    //bit 2 doesn't get shifted

    unsigned short resultU = bit1 | bit2;
    //unsigned int resultU = bit3 | bit4;
    short result = static_cast<short>(resultU);
    return result;
}

/*
void pushToEnd(vector toShift, int index) //not useable in this state, but can be implemented into other functions
{
    type toPush = toShift[index];
    for(int i = index; i < toShift.size(); i++)
    {
        if(i + 1 < toShift.size())
        {
            toShift[i] = toShift[i+1];
        }
        else
        {
            toShift[i] = toPush;
        }
    }
}
*/