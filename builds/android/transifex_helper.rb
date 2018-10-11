#!/bin/ruby
#
# joins the text files used by fastlane/supply to a XML file
# which can be uploaded to transifex or splits this file.
# by carstene1ns 2017, released under the MIT license

# these libraries are provided by ruby, so no external dependencies
require "rexml/document"
require 'optparse'

# default options
metadata_dir = "metadata"
lang_code = "en-US"
mode = ""

optparse = OptionParser.new do |opts|
  opts.banner = "Usage: transifex_helper.rb [options]"
  opts.separator ''
  opts.on("-m", "--mode MODE", "Set operation mode: split, join", String) do |m|
    mode = m
  end
  opts.on("-l", "--language LANG", "Set metadata language (Default is \"#{lang_code}\")", String) do |l|
    lang_code = l
  end
  opts.on("-d", "--directory DIR", "Set metadata directory (Default is \"#{metadata_dir}\")", String) do |d|
    metadata_dir = d
  end
  opts.on_tail("-h", "--help", "Show this message") do
    puts optparse
    exit
  end
end

begin
  optparse.parse!

  if mode == ""
    puts "Need to provide an operation mode!"
    puts optparse
    exit 1
  end

  # helper
  folder = File.join(metadata_dir, lang_code)

  def abort_no_file(file_name)
    puts "File \"#{file_name}\" not found, aborting!"
    exit 1
  end

  if mode.casecmp("join") == 0
    doc = REXML::Document.new("<resources></resources>")
    doc.context[:attribute_quote] = :quote # set double-quote as the attribute value delimiter
    doc << REXML::XMLDecl.new(version = 1.0, encoding = "utf-8")

    doc.root << REXML::Comment.new("Title (30 chars max)")
    title = doc.root.add_element "string", {"name" => "title"}
    file_name = File.join(folder, "title.txt")
    abort_no_file(file_name) if not File.exists?(file_name)
    File.open(file_name, "r") do |f|
      title.text = f.read
    end

    doc.root << REXML::Comment.new("Short description (80 chars max)")
    short_desc = doc.root.add_element "string", {"name" => "short_description"}
    file_name = File.join(folder, "short_description.txt")
    abort_no_file(file_name) if not File.exists?(file_name)
    File.open(file_name, "r") do |f|
      short_desc.text = f.read
    end

    doc.root << REXML::Comment.new("Long description (4000 chars max, linebreaks supported)")
    full_desc = doc.root.add_element "string", {"name" => "full_description"}
    file_name = File.join(folder, "full_description.txt")
    abort_no_file(file_name) if not File.exists?(file_name)
    File.open(file_name, "r") do |f|
      lines = f.read
      lines.gsub!(/\n/, '\n')
      full_desc.text = lines
    end

    # need our own formatting to preserve newlines
    xml = ""
    formatter = REXML::Formatters::Pretty.new(4)
    formatter.compact = true
    formatter.width = 4000 # maximum description width
    formatter.write(doc, xml)
    xml.gsub!('\n', "\n")

    file_name = File.join(folder, "playstore.xml")
    out_file = File.open(file_name, "w")
    out_file.write(xml)

    puts "Wrote \"#{file_name}\"."

  elsif mode.casecmp("split") == 0
    file_name = File.join(folder, "playstore.xml")
    abort_no_file(file_name) if not File.exists?(file_name)
    doc = REXML::Document.new(File.read(file_name))
    doc.get_elements('//string').each do |e|
      file_name = File.join(folder, e.attributes["name"] + ".txt")
      File.open(file_name, "w") do |f|
        f.write(e.text)
        puts "Wrote \"#{file_name}\"."
      end
    end

  else
    puts "Unknown operation mode: \"#{mode}\"."
    exit 1
  end
end
