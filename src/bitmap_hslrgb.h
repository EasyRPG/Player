
static inline void RGB_to_HSL(const uint8_t& r, const uint8_t& g, const uint8_t& b,
							  int &h, int &s, int &l)
{
	enum RGBOrder {
		O_RGB,
		O_RBG,
		O_GRB,
		O_GBR,
		O_BRG,
		O_BGR
	} order = (r > g)
		  ? ((r > b) ? ((g < b) ? O_RBG : O_RGB) : O_BRG)
		  : ((r < b) ? ((g > b) ? O_GBR : O_BGR) : O_GRB);

	int c = 0;
	int l2 = 0;
	switch (order) {
		case O_RGB: c = (r - b); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x000; l2 = (r + b); break;
		case O_RBG: c = (r - g); h = (c == 0) ? 0 : 0x100*(g - b)/c + 0x600; l2 = (r + g); break;
		case O_GRB: c = (g - b); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + b); break;
		case O_GBR: c = (g - r); h = (c == 0) ? 0 : 0x100*(b - r)/c + 0x200; l2 = (g + r); break;
		case O_BRG: c = (b - g); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + g); break;
		case O_BGR: c = (b - r); h = (c == 0) ? 0 : 0x100*(r - g)/c + 0x400; l2 = (b + r); break;
	}

	if (l2 == 0) {
		s = 0;
		l = 0;
	}
	else {
		s = 0x100 * c / ((l2 > 0xFF) ? 0x1FF - l2 : l2);
		l = l2 / 2;
	}
}

static inline void HSL_to_RGB(const int& h, const int& s, const int& l,
							  uint8_t &r, uint8_t &g, uint8_t &b)
{

	int l2 = 2 * l;
	int c = s * ((l2 > 0xFF) ? 0x1FF - l2 : l2) / 0x100;
	int m = (l2 - c) / 2;
	int h0 = h & 0xFF;
	int h1 = 0xFF - h0;

	switch (h >> 8) {
		case 0: r = m + c; g = m + h0*c/0x100; b = m; break;
		case 1: r = m + h1*c/0x100; g = m + c; b = m; break;
		case 2: r = m; g = m + c; b = m + h0*c/0x100; break;
		case 3: r = m; g = m + h1*c/0x100; b = m + c; break;
		case 4: r = m + h0*c/0x100; g = m; b = m + c; break;
		case 5: r = m + c; g = m; b = m + h1*c/0x100; break;
	}
}

static inline void HSL_adjust(int& h, int& s, int& l, int hue) {
	h += hue;
	if (h >= 0x600) h -= 0x600;
	if (s > 0xFF) s = 0xFF;
	l = (l > 0xFF) ? 0xFF : (l < 0) ? 0 : l;
}

static inline void RGB_adjust_HSL(uint8_t& r, uint8_t& g, uint8_t& b,
								  int hue) {
	int h, s, l;
	RGB_to_HSL(r, g, b, h, s, l);
	HSL_adjust(h, s, l, hue);
	HSL_to_RGB(h, s, l, r, g, b);
}
