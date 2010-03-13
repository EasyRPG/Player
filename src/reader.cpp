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
/// Read flag
////////////////////////////////////////////////////////////
bool Reader::Flag(FILE* stream) {
    return CInteger(stream) > 0;
}

////////////////////////////////////////////////////////////
/// Read BitFlag
////////////////////////////////////////////////////////////
unsigned char Reader::BitFlag(FILE* stream) {
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
    delete chars;
    return str;
}

////////////////////////////////////////////////////////////
/// Read array of uncompressed integers
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
/// Read array of uncompressed flags
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
