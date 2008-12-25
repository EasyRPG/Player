/*CDeltaTime.h, EasyRPG player CDeltaTime  class declaration file.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef CDELTATIME_H
#define CDELTATIME_H

class CDeltaTime {

	public:
		CDeltaTime(int pIdealFPS);
		void setIdealFPS(int pIdealFPS);
		void clear(); 
		void update();
		float deltaTime;
	private:
		long  idealFPS;
		float idealTime;
		float  timePrevious;
		float  timeCurrent;      
		float deltaTimeArray[16];
		int  deltaCurrentVector;
};

#endif
