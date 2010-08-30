//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "reader.h"

////////////////////////////////////////////////////////////
/// Constructor. Opens the file specified by filename.
////////////////////////////////////////////////////////////
Reader::Reader(char* filename)
{
    stream = fopen(filename, "rb");
}

////////////////////////////////////////////////////////////
/// Constructor. Opens the file specified by filename.
////////////////////////////////////////////////////////////
Reader::Reader(std::string filename)
{
    stream = fopen(filename.c_str(), "rb");
}

////////////////////////////////////////////////////////////
/// Destructor. Closes the opened file.
////////////////////////////////////////////////////////////
Reader::~Reader()
{
    fclose(stream);
}

////////////////////////////////////////////////////////////
/// Reads a compressed integer and checks if it is > 0
////////////////////////////////////////////////////////////
bool Reader::ReadBool()
{
    return (Read32(Reader::CompressedInteger) > 0);
}

////////////////////////////////////////////////////////////
/// Reads a 8bit value
////////////////////////////////////////////////////////////
uint8_t  Reader::Read8()
{
    uint8_t val;
    fread(&val, 1, 1, stream);
    return val;
}

////////////////////////////////////////////////////////////
/// Reads a 16bit value
////////////////////////////////////////////////////////////
int16_t  Reader::Read16()
{
    int16_t val;
    fread(&val, 2, 1, stream);
    return val;
}

////////////////////////////////////////////////////////////
/// Reads a 32bit value (Compressed or Uncompressed)
////////////////////////////////////////////////////////////
int32_t  Reader::Read32(IntegerType type)
{
    int32_t value = 0;
    unsigned char temp = 0;

    switch (type)
    {
    case Reader::NormalInteger:
        fread(&value, 4, 1, stream);
        return value;
        break;
    case Reader::CompressedInteger:
        do {
            value <<= 7;
            fread(&temp, 1, 1, stream); // Get byte's value
            value |= temp & 0x7F; // Check if it's a BER integer
        } while (temp & 0x80);

        return value;
        break;
    default:
        assert(false && "Invalid IntegerType in Read32");
        return 0;
        break;
    }
}

////////////////////////////////////////////////////////////
/// Reads 8bit values in a bool-array
////////////////////////////////////////////////////////////
void Reader::ReadBool(std::vector<bool>& buffer, size_t size)
{
    uint8_t val;
    for (unsigned i = 0; i < size; ++i)
    {
        fread(&val, 1, 1, stream);
        buffer.push_back(val > 0);
    }
}

////////////////////////////////////////////////////////////
/// Reads 8bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read8(std::vector<uint8_t>& buffer, size_t size)
{
    uint8_t val;
    for (unsigned i = 0; i < size; ++i)
    {
        fread(&val, 1, 1, stream);
        buffer.push_back(val);
    }
}

////////////////////////////////////////////////////////////
/// Reads 16bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read16(std::vector<int16_t>& buffer, size_t size)
{
    int16_t val;
	size_t items = size / 2;
    for (unsigned i = 0; i < items; ++i)
    {
        fread(&val, 2, 1, stream);
        buffer.push_back(val);
    }
}

////////////////////////////////////////////////////////////
/// Reads 32bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read32(std::vector<uint32_t>& buffer, size_t size)
{
    uint32_t val;
	size_t items = size / 4;
    for (unsigned i = 0; i < items; ++i)
    {
        fread(&val, 4, 1, stream);
        buffer.push_back(val);
    }
}

////////////////////////////////////////////////////////////
/// Reads a string
////////////////////////////////////////////////////////////
std::string Reader::ReadString(size_t size)
{
    char* chars = new char[size + 1];
    chars[size] = '\0';
    fread(chars, 1, size, stream);
    std::string str = std::string(chars, size);
    delete [] chars;
    return str;
}

////////////////////////////////////////////////////////////
/// Checks if the stream is initialized and no error occured
////////////////////////////////////////////////////////////
bool Reader::IsOk() const
{
    return (stream != NULL && !ferror(stream));
}

////////////////////////////////////////////////////////////
/// Checks if End of File has been reached
////////////////////////////////////////////////////////////
bool Reader::Eof() const
{
    return (feof(stream) != 0);
}

////////////////////////////////////////////////////////////
/// Seeks to a new location in the stream
////////////////////////////////////////////////////////////
void Reader::Seek(size_t pos, SeekMode mode)
{
    switch (mode)
    {
    case Reader::FromStart:
        rewind(stream);
        break;
    case Reader::FromCurrent:
        fseek(stream, pos, SEEK_CUR);
        break;
    case Reader::FromEnd:
        fseek(stream, pos, SEEK_END);
        break;
    default:
        assert(false && "Invalid SeekMode");
        break;
    }
}


/*
////////////////////////////////////////////////////////////
/// Read compressed integer
////////////////////////////////////////////////////////////
unsigned long Reader::CInteger(FILE* stream) {
    unsigned long value = 0;
    unsigned char temp = 0;

    do {
        value <<= 7;
        fread(&temp, sizeof(char), 1, stream); // Get byte's value
        value |= temp & 0x7F; // Check if it's a BER integer
    } while (temp & 0x80);

    return value;
}

////////////////////////////////////////////////////////////
/// Read short
////////////////////////////////////////////////////////////
short Reader::Short(FILE* stream) {
    short val;
    fread(&val, 2, 1, stream);
    return val;
}

////////////////////////////////////////////////////////////
/// Read Uint32
////////////////////////////////////////////////////////////
long Reader::Uint32(FILE* stream) {
    long val;
    fread(&val, 4, 1, stream);
    return val;
}

////////////////////////////////////////////////////////////
/// Read flag
////////////////////////////////////////////////////////////
bool Reader::Flag(FILE* stream) {// Read bool
    return CInteger(stream) > 0;
}

////////////////////////////////////////////////////////////
/// Read BitFlag
////////////////////////////////////////////////////////////
unsigned char Reader::BitFlag(FILE* stream) { // Read 8
    unsigned char val;
    fread(&val, 1, 1, stream);
    return val;
}

////////////////////////////////////////////////////////////
/// Read string
////////////////////////////////////////////////////////////
std::string Reader::String(FILE* stream, int lenght) {
    char* chars = new char[lenght + 1];
    memset(chars, 0, lenght + 1);
    fread(chars, sizeof(char), lenght, stream);
    std::string str = std::string(chars, lenght);
    delete [] chars;
    return str;
}

////////////////////////////////////////////////////////////
/// Read array of shorts
////////////////////////////////////////////////////////////
std::vector<short> Reader::ArrayShort(FILE* stream, int lenght) {
    std::vector<short> arr;
    short val;
    int items = lenght / 2;
    for (int i = 0; i < items; i++) {
        fread(&val, 2, 1, stream);
        arr.push_back(val);
    }
    return arr;
}

////////////////////////////////////////////////////////////
/// Read array of bools
////////////////////////////////////////////////////////////
std::vector<bool> Reader::ArrayFlag(FILE* stream, int lenght) {
    std::vector<bool> arr;
    unsigned char val;
    for (int i = 0; i < lenght; i++) {
        fread(&val, 1, 1, stream);
        arr.push_back(val > 0);
    }
    return arr;
}

////////////////////////////////////////////////////////////
/// Read array of Uint8
////////////////////////////////////////////////////////////
std::vector<unsigned char> Reader::ArrayUint8(FILE* stream, int lenght) {
    std::vector<unsigned char> arr;
    unsigned char val;
    for (int i = 0; i < lenght; i++) {
        fread(&val, 1, 1, stream);
        arr.push_back(val);
    }
    return arr;
}

////////////////////////////////////////////////////////////
/// Read array of Uint32
////////////////////////////////////////////////////////////
std::vector<unsigned long> Reader::ArrayUint32(FILE* stream, int lenght) {
    std::vector<unsigned long> arr;
    unsigned long val;
    int items = lenght / 4;
    for (int i = 0; i < items; i++) {
        fread(&val, 4, 1, stream);
        arr.push_back(val);
    }
    return arr;
}
*/