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

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

////////////////////////////////////////////////////////////
/// Output Namespace
////////////////////////////////////////////////////////////
namespace Output {
	void Post(char* fmt, ...);
	void Post(const char* fmt, ...);
	void PostStr(std::string msg);
	void PostFile(std::string msg);
	void Warning(char* fmt, ...);
	void Warning(const char* fmt, ...);
	void WarningStr(std::string warn);
	void Error(char* fmt, ...);
	void Error(const char* fmt, ...);
	void ErrorStr(std::string err);
}

#endif
