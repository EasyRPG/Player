raise "argument error" unless ARGV.length == 2

File.open(ARGV[1], "w").write("\xEF\xBB\xBF" + IO.read(ARGV[0]))
