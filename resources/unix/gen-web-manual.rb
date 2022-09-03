#!/bin/env ruby

# Builds the manual page for the homepage, enhanced with CSS and JS
# by carstene1ns, 2022

require 'asciidoctor'

class AddCustomCssAndJsProcessor < Asciidoctor::Extensions::DocinfoProcessor
  use_dsl
  at_location :header
  def process document
    %(<!-- EasyRPG modifications -->
<style>
html { scroll-behavior: smooth; }
h1, h2, h3, h4, h5, h6, #toctitle, .sidebarblock > .content > .title { color: #3C8025; }
body { background: #F5F5F5; }
#toc li a { transition: all 100ms ease-in-out; }
#toc li.active > a { font-weight: bolder; }
#toc li.active > a::before { content: "» "; }
</style>
<!-- Update TOC -->
<script>
window.addEventListener('DOMContentLoaded', () => {
  const observer = new IntersectionObserver(entries => {
    entries.forEach(entry => {
      const id = entry.target.firstChild.nextSibling.getAttribute('id');
      if (entry.intersectionRatio > 0) {
        document.querySelector(`#toc li a[href="#${id}"]`).parentElement.classList.add('active');
      } else {
        document.querySelector(`#toc li a[href="#${id}"]`).parentElement.classList.remove('active');
      }
    });
  });
  document.querySelectorAll('.sect1').forEach((section) => {
    observer.observe(section);
  });
});
</script>)
  end
end

class CustomHeaderFooterPostprocessor < Asciidoctor::Extensions::Postprocessor
  def process document, output
    version = document.attr 'player_version'
    # replace title
    content = %(EasyRPG Player #{version} Manual Page)
    replacement = %(<h1>#{content}</h1>)
    output = output.sub(/<h1>.*?<\/h1>/m, replacement)
    # add to footer
    content = %(EasyRPG Player #{version} © EasyRPG team)
    replacement = %(<div id="footer-text">\\1 - #{content}</div>)
    output = output.sub(/<div id="footer-text">(.*?)<\/div>/m, replacement)
    output
  end
end

# load extensions
Asciidoctor::Extensions.register do
  docinfo_processor AddCustomCssAndJsProcessor
  postprocessor CustomHeaderFooterPostprocessor
end

# use version argument
if ARGV.length == 1
  version = ARGV[0]
end

# convert!
Asciidoctor.convert_file File.join(__dir__, 'easyrpg-player.6.adoc'),
  safe: :unsafe, to_file: 'index.html',
  attributes: "toc=right player_version=#{version}"
