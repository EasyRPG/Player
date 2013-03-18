/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include "zobj.h"

ZObj::ZObj(int z, uint32_t creation, uint32_t ID)  :
	z(z),
	creation(creation),
	ID(ID) {
}

void ZObj::SetZ(int nz) {
	z = nz;
}
int ZObj::GetZ() const {
	return z;
}

uint32_t ZObj::GetCreation() const {
	return creation;
}

uint32_t ZObj::GetId() const {
	return ID;
}
