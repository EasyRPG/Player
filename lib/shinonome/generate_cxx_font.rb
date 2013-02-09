#!/usr/bin/env ruby

FONT_SIZE = 12
EMPTY_CHAR = Array.new(FONT_SIZE, 0x0)

require 'iconv'

def skip_until(f, regex)
  while(not f.eof?)
    l = f.readline()
    ret = regex.match(l)
    if ret
      return ret
    end
  end
  return false
end

def read_font(f, width)
  code = skip_until(f, /STARTCHAR (\w+)/)
  if not code
    return false
  else
    code = code[1].hex
  end

  skip_until(f, /BITMAP/)

  ret = Array.new(FONT_SIZE)

  for y in 0..(FONT_SIZE - 1)
    str = f.readline()

    tmp = 0
    for x in 0..(width - 1)
      if str[x] == ?@
        tmp |= 0x01 << x
      else
        raise "assert" unless str[x] == ?.
      end
    end
    ret[y] = tmp
  end

  raise "assert" unless /ENDCHAR/.match(Iconv.conv('UTF-8', 'EUC-JP', f.readline()))

  return [code, ret]
end

def read_half(f)
  ret = Array.new(0x100, nil)
  while(true)
    font = read_font(f, FONT_SIZE / 2)
    if not font
      return ret
    else
      ret[font[0]] = font[1]
    end
  end
end

def write_half(f, sym, fonts)
  f.write("#include \"shinonome.hxx\"\n")
  f.write("#include <boost/cstdint.hpp>\n\n")
  f.write("uint16_t const %s[0x100][%d] = {\n" % [sym, FONT_SIZE])
  raise "assert" unless fonts.length <= 0x100
  fonts.each_index {|idx|

    font = fonts[idx]
    f.write("  { ")

    if font == nil
      font = EMPTY_CHAR
    end

    for l in font
      f.write("0x%02x, " % [l])
    end

    f.write("}, // 0x%02x\n" % [idx])
  }
  f.write("}; // %s\n" % [sym])
end

def read_full(f)
  ret = Array.new(94, Array.new(94))
  while(true)
    font = read_font(f, FONT_SIZE)
    if not font
      return ret
    else
      ku  = ((font[0] - 0x2100) >> 8*1) & 0xff
      ten = ((font[0] - 0x0021) >> 8*0) & 0xff

      # raise "invalid ku: %d" % [ku] unless ku < 94
      # raise "invalid ten: %d" % [ten] unless ten < 94

      ret[ku][ten] = font[1]
    end
  end
end

def write_full(f, sym, fonts)
  f.write("#include \"shinonome.hxx\"\n")
  f.write("#include <boost/cstdint.hpp>\n\n")
  f.write("uint16_t const %s[94][94][%d] = {\n" % [sym, FONT_SIZE])
  raise "invalid ku max: %d" % [fonts.length-1] unless fonts.length <= 94
  fonts.each_index {|ku_idx|
    ku = fonts[ku_idx]
    raise "invalid ten max: %d" % [ku.length-1] unless ku.length <= 94

    f.write("  { // ku: %2d\n" % [ku_idx + 1])
    ku.each_index {|ten_idx|

      ten = ku[ten_idx]
      f.write("    { ")

      if ten == nil
        ten = EMPTY_CHAR
      end

      for l in ten
        f.write("0x%03x, " % [l])
      end

      f.write("}, // ten: %2d\n" % [ten_idx + 1])
    }
    f.write("  }, // ku: %2d\n" % [ku_idx + 1])
  }
  f.write("}; // %s\n" % [sym])
end

# header
print "Generating Header..."
header = File.new('./shinonome.hxx', 'w')
for l in
  [
   '#ifndef _INC_SHINONOME_HXX_',
   '#define _INC_SHINONOME_HXX_',
   '',
   '#include <boost/cstdint.hpp>',
   '',
   '',
   'extern "C" {',
   '',
   'extern uint16_t const SHINONOME_LATIN1[0x100][%d];' % FONT_SIZE,
   'extern uint16_t const SHINONOME_HANKAKU[0x100][%d];' % FONT_SIZE,
   '',
   'extern uint16_t const SHINONOME_GOTHIC[94][94][%d];' % FONT_SIZE,
   'extern uint16_t const SHINONOME_MINCHO[94][94][%d];' % FONT_SIZE,
   '',
   '}',
   '',
   '#endif // _INC_SHINONOME_HXX_',
   '',
  ]
  header.write(l + "\n");
end
print "done\n"

print "Genarating Lantin-1..."
latin = read_half(File.new('./latin1/font_src.bit', 'r'))
write_half(File.new('./latin1.cxx', 'w'), 'SHINONOME_LATIN1', latin)
print "done\n"

print "Generating Hankaku..."
hankaku = read_half(File.new('./hankaku/font_src_diff.bit', 'r'))
hankaku.each_index {|x|
  if hankaku[x] == nil
    hankaku[x] = latin[x]
  end
}
write_half(File.new('./hankaku.cxx', 'w'), 'SHINONOME_HANKAKU', hankaku)
print "done\n"

print "Genarating Gothic..."
gothic = read_full(File.new('./kanjic/font_src.bit', 'r'))
write_full(File.new('./gothic.cxx', 'w'), 'SHINONOME_GOTHIC', gothic)
print "done\n"

print "Generating Mincho..."
mincho = read_full(File.new('./mincho/font_src_diff.bit', 'r'))
for ku in 0..93
  if mincho[ku] == nil
    mincho[ku] = gothic[ku]
  else
    for ten in 0..93
      if mincho[ku][ten] == nil
        mincho[ku][ten] = gothic[ku][ten]
      end
    end
  end
end
write_full(File.new('./mincho.cxx', 'w'), 'SHINONOME_MINCHO', mincho)
print "done\n"
