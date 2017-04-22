#! /usr/bin/env python3

bdf = 'wenquanyi_cjk_basic_9pt.bdf'
cpp = '../../src/bitmapfont_wqy.cpp'
##cpp = 'bitmapfont_wqy.cpp'

# FONTBOUNDINGBOX 12 14 0 -3
control_point = 14 - 3
h = 12
w = 12
w_half = 12 // 2

chars = 0
glyphs = []

def getKey(item):
    return item[0]

if __name__ == '__main__':
    with open(bdf) as f:
        new_glyph = False
        glyph_line_count = 0
        glyph = []
        xoff = 0 # 左移位数

        for line in f:
            #print(line)
            if line.find('CHARS ') == 0: #CHARS 30503
                s = line.split()
                #print('CHARS: %s' % s)
                chars = int(s[1])
                #print(chars)

            elif line.find('STARTCHAR ') == 0:
                glyph = [0, 'true', ['0']*w]
                #print(glyph)

            elif line.find('ENCODING ') == 0:
                s = line.split()
                glyph[0] = int(s[1])

            elif line.find('BBX ') == 0:
                bbx = line.split()
                #print(bbx)
                # 计算偏移, 并将该字节左移.
                x_padding = int(bbx[1]) + int(bbx[3])
                if x_padding <= w_half:
                    glyph[1] = 'false'
                    xoff = w_half - x_padding
                else:
                    xoff = w - x_padding
                # 计算高度, 补全收尾行.
                y_padding = int(bbx[4]) + 2 # 底补全
                y_padding_first = h - int(bbx[2]) - y_padding # 顶补全 = 全长(12px) - 字高 - 底补全

                #if xoff < 0 or y_padding_first < 0 or y_padding < 0: print(xoff, y_padding, y_padding_first, glyph[0])
                #if xoff != 1 and y_padding_first != 0 and y_padding != 1: print(xoff, y_padding, y_padding_first, glyph[0])
                #print(glyph[0], ' BBX gen offsite :', xoff, y_padding_first, y_padding)

            elif line.find('BITMAP') == 0:
                new_glyph = True
                glyph_line_count = 0
                if y_padding_first > 0:
                    glyph_line_count = glyph_line_count + y_padding_first
                else:
                    pass # < 0, need skip some lines.

            elif line.find('ENDCHAR') == 0:
                new_glyph = False
                glyph_line_count = 0
                #print('Final Glyph is: %s.' % glyph)
                glyphs.append(glyph)

            elif new_glyph:
                # pack line
                line_final = ''
                line_s = line.strip()
                #print(line_s)
                i = int('0x%s' % line_s, 16) >> 4
                i_str = '{0:0>12b}'.format(i)
                i_str_new = i_str[11] + i_str[10] + i_str[9] + i_str[8] + i_str[7] + i_str[6] + i_str[5] + i_str[4] + i_str[3] + i_str[2] + i_str[1] + i_str[0]
                i_int_new = int('0b%s' % i_str_new, 2)
                #print('{0:0>12b}'.format(i_int_new))
                #if xoff > 0: pass
                #if xoff < 0: pass
                #print('glyph_line_count', glyph_line_count)
                glyph[2][glyph_line_count] = str(i_int_new)
                glyph_line_count = glyph_line_count + 1

    glyphs.sort(key=getKey)

    fw = open(cpp, 'w')
    fw.write('''#include "bitmapfont.h"

BitmapFontGlyph const BITMAPFONT_WQY[%s] = {
''' % len(glyphs))
    for x in glyphs:
        #print(x[0])
        #print(', '.join(x[2]))
        s = '\t{ %s, %s, { %s } },\n' % (x[0], x[1], ', '.join(x[2]))
        #print(s)
        fw.write(s)

    fw.write('};\n')
    fw.close()

