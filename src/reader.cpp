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

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "reader.h"
#include <cstdarg>

////////////////////////////////////////////////////////////
/// Statics
////////////////////////////////////////////////////////////
std::string Reader::error_str;

////////////////////////////////////////////////////////////
/// Constructor. Opens the file specified by filename.
////////////////////////////////////////////////////////////
Reader::Reader(char* filename) {
	stream = fopen(filename, "rb");
}

////////////////////////////////////////////////////////////
/// Constructor. Opens the file specified by filename.
////////////////////////////////////////////////////////////
Reader::Reader(const std::string& filename) {
	stream = fopen(filename.c_str(), "rb");
}

////////////////////////////////////////////////////////////
/// Destructor. Closes the opened file.
////////////////////////////////////////////////////////////
Reader::~Reader() {
	if (stream != NULL) {
		fclose(stream);
	}
}

////////////////////////////////////////////////////////////
/// Reads a compressed integer and checks if it is > 0
////////////////////////////////////////////////////////////
bool Reader::ReadBool() {
	return (Read32(Reader::CompressedInteger) > 0);
}

////////////////////////////////////////////////////////////
/// Reads a 8bit value
////////////////////////////////////////////////////////////
uint8_t Reader::Read8() {
	uint8_t val = 0;
#ifndef NDEBUG
	assert(fread(&val, 1, 1, stream) == 1);
#else
	fread(&val, 1, 1, stream);
#endif
	return val;
}

////////////////////////////////////////////////////////////
/// Reads a 16bit value
////////////////////////////////////////////////////////////
int16_t Reader::Read16() {
	int16_t val = 0;
#ifndef NDEBUG
	assert(fread(&val, 2, 1, stream) == 1);
#else
	fread(&val, 2, 1, stream);
#endif
	return val;
}

////////////////////////////////////////////////////////////
/// Reads a 32bit value (Compressed or Uncompressed)
////////////////////////////////////////////////////////////
int32_t Reader::Read32(IntegerType type) {
	int32_t value = 0;
	unsigned char temp = 0;

	switch (type) {
	case Reader::NormalInteger:
#ifndef NDEBUG
		assert(fread(&value, 4, 1, stream) == 1);
#else
		fread(&value, 4, 1, stream);
#endif
		return value;
	case Reader::CompressedInteger:
		do {
			value <<= 7;
			if (fread(&temp, 1, 1, stream) != 1) // Get byte's value
				return 0;
			value |= temp & 0x7F; // Check if it's a BER integer
		} while (temp & 0x80);

		return value;
	default:
#ifndef NDEBUG
		assert(false && "Invalid IntegerType in Read32");
#endif
		return 0;
	}
}

////////////////////////////////////////////////////////////
/// Reads 8bit values in a bool-array
////////////////////////////////////////////////////////////
void Reader::ReadBool(std::vector<bool> &buffer, size_t size) {
	uint8_t val = 0;
	for (unsigned i = 0; i < size; ++i) {
#ifndef NDEBUG
		assert(fread(&val, 1, 1, stream) == 1);
#else
		fread(&val, 1, 1, stream);
#endif
		buffer.push_back(val > 0);
	}
}

////////////////////////////////////////////////////////////
/// Reads 8bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read8(std::vector<uint8_t> &buffer, size_t size) {
	uint8_t val;
	for (unsigned int i = 0; i < size; ++i) {
#ifndef NDEBUG
		assert(fread(&val, 1, 1, stream) == 1);
#else
		fread(&val, 1, 1, stream);
#endif
		buffer.push_back(val);
	}
}

////////////////////////////////////////////////////////////
/// Reads 16bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read16(std::vector<int16_t> &buffer, size_t size) {
	int16_t val;
	size_t items = size / 2;
	for (unsigned int i = 0; i < items; ++i) {
#ifndef NDEBUG
		assert(fread(&val, 2, 1, stream) == 1);
#else
		fread(&val, 2, 1, stream);
#endif
		buffer.push_back(val);
	}
}

////////////////////////////////////////////////////////////
/// Reads 32bit values in an array
////////////////////////////////////////////////////////////
void Reader::Read32(std::vector<uint32_t> &buffer, size_t size) {
	uint32_t val;
	size_t items = size / 4;
	for (unsigned int i = 0; i < items; ++i) {
#ifndef NDEBUG
		assert(fread(&val, 4, 1, stream) == 1);
#else
		fread(&val, 4, 1, stream);
#endif
		buffer.push_back(val);
	}
}

////////////////////////////////////////////////////////////
/// Reads a string
////////////////////////////////////////////////////////////
std::string Reader::ReadString(size_t size) {
	char* chars = new char[size + 1];
	chars[size] = '\0';
#ifndef NDEBUG
	assert(fread(chars, 1, size, stream) == size);
#else
	fread(chars, 1, size, stream);
#endif
	std::string str = std::string(chars, size);
	delete [] chars;
	return str;
}

////////////////////////////////////////////////////////////
/// Checks if the stream is initialized and no error occured
////////////////////////////////////////////////////////////
bool Reader::IsOk() const {
	return (stream != NULL && !ferror(stream));
}

////////////////////////////////////////////////////////////
/// Checks if End of File has been reached
////////////////////////////////////////////////////////////
bool Reader::Eof() const {
	return feof(stream) != 0;
}

////////////////////////////////////////////////////////////
/// Seeks to a new location in the stream
////////////////////////////////////////////////////////////
void Reader::Seek(size_t pos, SeekMode mode) {
	switch (mode) {
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
#ifndef NDEBUG
		assert(false && "Invalid SeekMode");
#else
		;
#endif
	}
}

////////////////////////////////////////////////////////////
/// Puts the last read character (ch) back in the buffer
////////////////////////////////////////////////////////////
bool Reader::Ungetch(uint8_t ch) {
	return (ungetc(ch, stream) == ch);
}

////////////////////////////////////////////////////////////
/// Set Error string
////////////////////////////////////////////////////////////
void Reader::SetError(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char str[256];
	vsprintf(str, fmt, args);

	error_str = str;
	//Output::ErrorStr((std::string)str);

	va_end(args);
	//error_str = str;
}

////////////////////////////////////////////////////////////
/// Get Error string
////////////////////////////////////////////////////////////
const std::string& Reader::GetError() {
	return error_str;
}