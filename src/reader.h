/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _READER_H_
#define _READER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdio.h>
#ifndef NDEBUG
#include <assert.h>
#endif
#ifndef _MSC_VER
	#include <stdint.h>
#else
	typedef	unsigned char	uint8_t;
	typedef	signed short	int16_t;
	typedef unsigned short	uint16_t;
	typedef	signed int		int32_t;
	typedef unsigned int	uint32_t;
#endif

////////////////////////////////////////////////////////////
/// Reader class
////////////////////////////////////////////////////////////
class Reader {
public:
	Reader(char* filename);
	Reader(const std::string& filename);
	~Reader();

	static const std::string& GetError();
	static void SetError(const char* fmt, ...);

	struct Chunk {
		Chunk() {
			ID = 0;
			length = 0;
		}
		uint32_t ID;
		uint32_t length;
	};

	enum IntegerType {
		CompressedInteger,
		NormalInteger
	};

	enum SeekMode {
		FromStart,
		FromEnd,
		FromCurrent
	};

	bool ReadBool();
	uint8_t Read8();
	int16_t Read16();
	int32_t Read32(IntegerType type);
	void ReadBool(std::vector<bool> &buffer, size_t size);
	void Read8(std::vector<uint8_t> &buffer, size_t size);
	void Read16(std::vector<int16_t> &buffer, size_t size);
	void Read32(std::vector<uint32_t> &buffer, size_t size);
	std::string ReadString(size_t size);
	bool IsOk() const;
	bool Eof() const;
	void Seek(size_t pos, SeekMode = FromStart);
	bool Ungetch(uint8_t ch);

private:
	FILE* stream;
	static std::string error_str;
};

#endif
