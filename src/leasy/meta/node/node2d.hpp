/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

#pragma once

#include "node.hpp"
#include "types.hpp"

namespace leasy::meta2::node {
	class Node2D : public Node {
	protected:
		pos_t x, y;

	public:
		struct position {
			pos_t x, y;
			inline position() = default;
			inline position(const pos_t &x, const pos_t &y) : x(x), y(y) {}
			inline std::tuple<pos_t, pos_t> tuple() {
				return {this->x, this->y};
			}
		};

		inline Node2D() : x(0), y(0) {}
		inline Node2D(const pos_t &x, const pos_t &y) : x(x), y(y) {}
		inline position pos() const { return {this->x, this->y}; }
		inline void move(pos_t x, pos_t y) { this->x = x; this->y = y; }
	};
}
