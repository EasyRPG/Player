#!/usr/bin/env ruby -Ku
# encoding: utf-8
raise "argument error" unless ARGV.length == 2

File.open(ARGV[1], "w").write("\xEF\xBB\xBF" + IO.read(ARGV[0]))
