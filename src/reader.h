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

#ifndef _READER_H_
#define _READER_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#ifndef _MSC_VER
    #include <stdint.h>
#else
    typedef unsigned char  uint8_t;
    typedef   signed short  int16_t;
    typedef unsigned short uint16_t;
    typedef   signed int    int32_t;
    typedef unsigned int   uint32_t;
#endif

////////////////////////////////////////////////////////////
/// Reader namespace
////////////////////////////////////////////////////////////
/*namespace Reader {
    struct Chunk {
        unsigned long ID;
        unsigned long length;
    };
    unsigned long CInteger(FILE* stream);
    short Short(FILE* stream);
    long Uint32(FILE* stream);
    bool Flag(FILE* stream);
    unsigned char BitFlag(FILE* stream);
    std::string String(FILE* stream, int lenght);
    std::vector<short> ArrayShort(FILE* stream, int lenght);
    std::vector<unsigned char> ArrayUint8(FILE* stream, int lenght);
    std::vector<unsigned long> ArrayUint32(FILE* stream, int lenght);
    std::vector<bool> ArrayFlag(FILE* stream, int lenght);
}*/
class Reader {
public:
    Reader(char* filename);
    Reader(std::string filename);
    ~Reader();

    struct Chunk {
        uint32_t ID;
        uint32_t length;
    };

    enum IntegerType
    {
        CompressedInteger,
        NormalInteger
    };

    enum SeekMode
    {
        FromStart,
        FromEnd,
        FromCurrent
    };

    bool     ReadBool();
    uint8_t  Read8();
    int16_t  Read16();
    int32_t  Read32(IntegerType type);
    void ReadBool(std::vector<bool>& buffer, size_t size);
    void Read8(std::vector<uint8_t>& buffer, size_t size);
    void Read16(std::vector<int16_t>& buffer, size_t size);
    void Read32(std::vector<uint32_t>& buffer, size_t size);
    std::string ReadString(size_t size);
    bool IsOk() const;
    bool Eof() const;
    void Seek(size_t pos, SeekMode = FromStart);
    /*unsigned long CInteger(FILE* stream);
    short Short(FILE* stream);
    long Uint32(FILE* stream);
    bool Flag(FILE* stream);
    unsigned char BitFlag(FILE* stream);
    std::string String(FILE* stream, int length);
    std::vector<short> ArrayShort(FILE* stream, int length);
    std::vector<unsigned char> ArrayUint8(FILE* stream, int length);
    std::vector<unsigned long> ArrayUint32(FILE* stream, int length);
    std::vector<bool> ArrayFlag(FILE* stream, int length);*/

private:
    FILE* stream;
};

#endif
