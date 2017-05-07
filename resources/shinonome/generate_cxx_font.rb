#!/usr/bin/env ruby
# encoding: utf-8
$KCODE = 'UTF8' unless RUBY_VERSION > "1.9.0"

FONT_SIZE = 12
EMPTY_CHAR = Array.new(FONT_SIZE, 0x0)

def skip_until(f, regex)
  while(not f.eof?)
    l = f.readline().chomp
    ret = regex.match(l)
    if ret
      return ret
    end
  end
  return false
end

class Glyph
  attr_reader :code, :is_full, :data

  def initialize(c, f, d)
    raise "type error" unless c.instance_of? Fixnum
    raise "type error" unless d.instance_of? Array

    @code = c
    @is_full = f
    @data = d
  end
end


def read_font(f, half, encoding)
  code = skip_until(f, /STARTCHAR\s+(\w+)/)
  if not code
    return false
  else
    c = code[1].hex
    if encoding == "JIS_X0208"
      encoding = "CP932"

      j1 = (c >> 8) & 0xff
      j2 = c & 0xff

      s1 = (32 < j1 and j1 <= 94) ? (j1 + 1) / 2 + 112 : (j1 + 1) / 2 + 176
      s2 = (j1 & 0x01) == 1 ? j2 + 31 + j2 / 96 : j2 + 126

      c = ((s1 & 0xff) << 8) + s2
      code = [s1, s2].pack("CC")
    elsif encoding == "UTF-32LE"
      code = [c].pack("L<")
    else
      code = (c < 0x100 ? [c] : [c & 0xff, (c >> 8) & 0xff]).pack("C*")
    end

    raise "size error" unless code.bytesize == (c < 0x100 ? 1 : 2) || encoding == "UTF-32LE"

    begin
      code = code.force_encoding(encoding).encode('UTF-32LE')
    rescue Encoding::InvalidByteSequenceError
      print "invalid code 0x%02x, 0x%02x\n" % [(c >> 8) & 0xff, c & 0xff]
      return false
    end

    raise "invalid code" unless code.bytesize == 4

    code = code.unpack("V")[0]
  end

  width = half ? FONT_SIZE / 2 : FONT_SIZE

  skip_until(f, /BITMAP/)

  ret = Array.new(FONT_SIZE, 0)

  for y in 0...FONT_SIZE
    str = f.readline()

    tmp = 0
    for x in 0...width
      if str[x] == ?@
        tmp |= 0x01 << x
      else
        raise "assert" unless str[x] == ?.
      end
    end
    ret[y] = tmp
  end

  raise "assert" unless /ENDCHAR/.match(f.readline().encode('UTF-8', 'EUC-JP'))

  return Glyph.new(code, !half, ret)
end

def read_file(f, encoding, half)
  ret = {}
  while(true)
    font = read_font(f, half, encoding)
    if f.eof?
      return ret
    elsif font
      ret[font.code] = font
    end
  end
end

def write_all(f, sym, data)
  f.write <<EOS
#include "bitmapfont.h"

BitmapFontGlyph const #{sym}[#{data.size}] = {
EOS

  code_max = 0

  data.sort.each { |v|
    g = v[1]
    f.write "  { #{g.code}, #{g.is_full}, { #{g.data.join(", ")} } },\n"
    code_max = [g.code, code_max].max
  }
  f.write "};\n"

  code_max
end

def read_bdf_chars(f)
  code = skip_until(f, /CHARS\s+(\d+)/)
  raise "assert" unless code
  return code[1]
end

# loading
print "Loading Latin-1..."
latin = read_file(File.new('./latin1/font_src.bit', 'r'), "ISO-8859-1", true)
print "done\n"

print "Loading Latin Extended A..."
latin_ext_a = read_file(File.new('./latin-ext-a/font_src.bit', 'r'), "UTF-32LE", true)
print "done\n"

print "Loading Extras..."
extras = read_file(File.new('./extras/font_src.bit', 'r'), "UTF-32LE", true)
print "done\n"

print "Loading Extras (Fullwidth)..."
extras_fullwidth = read_file(File.new('./extras-fullwidth/font_src.bit', 'r'), "UTF-32LE", false)
print "done\n"

print "Loading Hankaku..."
hankaku = read_file(File.new('./hankaku/font_src_diff.bit', 'r'), "CP932", true)
print "done\n"

print "Loading Gothic..."
gothic = read_file(File.new('./kanjic/font_src.bit', 'r'), "JIS_X0208", false)
print "done\n"

print "Loading Mincho..."
mincho = read_file(File.new('./mincho/font_src_diff.bit', 'r'), "JIS_X0208", false)
print "done\n"

print "Loading Korean..."
korean = read_file(File.new('./korean/font_src_diff.bit', 'r'), "UTF-32LE", false)
print "done\n"

print "Loading Chinese..."
chinese = read_file(File.new('./chinese/font_src_diff.bit', 'r'), "UTF-32LE", false)
print "done\n"

print "Loading RMG2000..."
rmg2000 = read_file(File.new('./rmg2000/font_src.bit', 'r'), "UTF-32LE", true)
print "done\n"

print "Loading ttyp0 (RM2000 replacement)..."
ttyp0 = read_file(File.new('../ttyp0/font.bit', 'r'), "UTF-32LE", true)
print "done\n"

print "Loading WenQuanYi..."
wenquanyi_chars = read_bdf_chars(File.new('../wenquanyi/wenquanyi_cjk_basic_9pt.bdf', 'r'))
print "done\n"

# generating
print "Generating Gothic..."
gothic_final = gothic.merge(hankaku) \
	.merge(korean).merge(chinese).merge(latin) \
    .merge(latin_ext_a).merge(extras).merge(extras_fullwidth)
code_max = write_all(File.new("../../src/shinonome_gothic.cpp", "w"), "SHINONOME_GOTHIC", gothic_final)
print "done\n"

print "Generating Mincho..."
code_max = [write_all(File.new("../../src/shinonome_mincho.cpp", "w"), "SHINONOME_MINCHO", mincho), code_max].max
print "done\n"

print "Generating RMG2000..."
code_max = [write_all(File.new("../../src/bitmapfont_rmg2000.cpp", "w"), "BITMAPFONT_RMG2000", rmg2000), code_max].max
print "done\n"

print "Generating ttyp0..."
code_max = [write_all(File.new("../../src/bitmapfont_ttyp0.cpp", "w"), "BITMAPFONT_TTYP0", ttyp0), code_max].max
print "done\n"

# header
print "Generating Header..."
File.new('../../src/bitmapfont.h', 'w').write <<EOS
#ifndef _INC_BITMAPFONT_H_
#define _INC_BITMAPFONT_H_

#include <stdint.h>

struct BitmapFontGlyph {
	uint#{code_max < 0x10000 ? 16 : 32}_t code;
	bool is_full;
	uint16_t data[#{FONT_SIZE}];
};

extern BitmapFontGlyph const SHINONOME_GOTHIC[#{gothic_final.size}];
extern BitmapFontGlyph const SHINONOME_MINCHO[#{mincho.size}];
extern BitmapFontGlyph const BITMAPFONT_WQY[#{wenquanyi_chars}];
extern BitmapFontGlyph const BITMAPFONT_RMG2000[#{rmg2000.size}];
extern BitmapFontGlyph const BITMAPFONT_TTYP0[#{ttyp0.size}];

#endif // _INC_BITMAPFONT_H_
EOS
print "done\n"
