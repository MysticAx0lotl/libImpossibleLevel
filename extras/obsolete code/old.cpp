
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

/*
int swapIntEndians(int value) 
{ 
 
    // This var holds the leftmost 8 
    // bits of the output. 
    int leftmost_byte; 
 
    // This holds the left middle 
    // 8 bits of the output 
    int left_middle_byle; 
 
    // This holds the right middle 
    // 8 bits of the output 
    int right_middle_byte; 
 
    // This holds the rightmost 
    // 8 bits of the output 
    int rightmost_byte; 
 
    // To store the result 
    // after conversion 
    int result; 
 
    // Get the rightmost 8 bits of the number 
    // by anding it 0x000000FF. since the last 
    // 8 bits are all ones, the result will be the 
    // rightmost 8 bits of the number. this will 
    // be converted into the leftmost 8 bits for the 
    // output (swapping) 
    leftmost_byte = (value & 0x000000FF) >> 0; 
 
    // Similarly, get the right middle and left 
    // middle 8 bits which will become 
    // the left_middle bits in the output 
    left_middle_byle = (value & 0x0000FF00) >> 8; 
 
    right_middle_byte = (value & 0x00FF0000) >> 16; 
 
    // Get the leftmost 8 bits which will be the 
    // rightmost 8 bits of the output 
    rightmost_byte = (value & 0xFF000000) >> 24; 
 
    // Left shift the 8 bits by 24 
    // so that it is shifted to the 
    // leftmost end 
    leftmost_byte <<= 24; 
 
    // Similarly, left shift by 16 
    // so that it is in the left_middle 
    // position. i.e, it starts at the 
    // 9th bit from the left and ends at the 
    // 16th bit from the left 
    left_middle_byle <<= 16; 
 
    right_middle_byte <<= 8; 
 
    // The rightmost bit stays as it is 
    // as it is in the correct position 
    rightmost_byte <<= 0; 
 
    // Result is the concatenation of all these values. 
    result = (leftmost_byte | left_middle_byle |
              right_middle_byte | rightmost_byte); 
 
    return result; 
} 

int swapShortEndians(short value) 
{ 
    short leftmost_byte; 

    short rightmost_byte; 

    short result; 
 
    // Get the rightmost 8 bits of the number 
    // by anding it 0x000000FF. since the last 
    // 8 bits are all ones, the result will be the 
    // rightmost 8 bits of the number. this will 
    // be converted into the leftmost 8 bits for the 
    // output (swapping) 
    leftmost_byte = (value & 0x000000FF) >> 0;
 
    // Get the leftmost 8 bits which will be the 
    // rightmost 8 bits of the output 
    rightmost_byte = (value & 0xFF000000) >> 8; 
 
    // Left shift the 8 bits by 24 
    // so that it is shifted to the 
    // leftmost end 
    leftmost_byte <<= 8; 
 
    // The rightmost bit stays as it is 
    // as it is in the correct position 
    rightmost_byte <<= 0; 
 
    // Result is the concatenation of all these values. 
    result = (leftmost_byte | rightmost_byte); 
 
    return result; 
} */

//Source: https://codereview.stackexchange.com/a/22907
//modified to convert to and then return an unsigned char vector instead of a signed one

/*
static std::vector<unsigned char> ReadAllBytes(const std::string& filename)
{
    std::ifstream ifs(filename.c_str(), std::ios::binary|std::ios::ate);
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
*/