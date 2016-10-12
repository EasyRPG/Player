#!/usr/bin/env perl
# $Id: bdfmerge,v 1.5 2006/12/12 20:31:23 fangq Exp $
#
# bdfmerge: merge two or more BDFs into one file,
#           sorting the characters by encodings.
#
#           the new version allows users to specify 
#           encoding ranges (can be single character,
#           one or multiple ranges)
#
#   USAGE: bdfmerge [range1 range2 ...] bdf1 bdf2 ... > out_bdf
#
#   NOTICE: The header of the 1st BDF file is used 
#           (from the beginning of file ... ENDPROPERTIES).
#           Other headers are simply ignored.
#
#           character ranges have format like
#           0x4E00 (for single char.) or 0x4E00--0x9FA5
#
#   EXAMPLE: bdfmerge 0x20--0x75 0x4E00--0x9FA5 0xFFEE wenquanyi_12pt.bdf
#
#   2006/12/11 by FangQ (fangq <at> gmail <dot> com)
#   2002/5/14, by 1@2ch
#   * public domain*
#

$inheader = 1;

my %goodchar;

while(@ARGV>=1&& $ARGV[0]=~/^-{0,1}0[xX]([0-9A-Fa-f]{0,6})/)
{
	$startpos = hex($1);
	$endpos=0;
	$range=shift(ARGV);
	if($range=~/--0[xX]([0-9A-Fa-f]{0,6})/)
	{
		$endpos=hex($1);
	}
	if($endpos>=$startpos) 
	{
		for($i=$startpos;$i<=$endpos;$i++)
		{
			$goodchar{$i}=($range=~/^-/?0:1);
		}
	}
	else
	{
		$goodchar{$startpos}++;
	}
}
$goodchar{32}=1; #add space
$goodchar{0}=1;  #add default fallback

while(<>) {
    if (/^startchar\b/i) {
	$inchar = 1; 
	$char1 = '';
    }
    if ($inchar && /^encoding\s+([-0-9]+)\b/i) {
	$enc = $1;
    }
    if ($inchar) {
	$char1 .= $_;
    } elsif ($inheader) {
	# skip CHARS
	print $_ unless (/^chars\b/i)
    }
    if (/^endchar/i) {
	$inchar = 0;
	$chars{$enc} = $char1 if (0 < $enc && $goodchar{$enc} || $enc==$dchar);
    }
    $inheader = 0 if (/^endproperties\b/i);
}

# CHARS tag immediately follows ENDPROPERTIES
print "CHARS ", 0+keys(%chars), "\n";

# flush buffers
foreach $i (sort {$a<=>$b} keys(%chars)) {
    print $chars{$i};
}

# ENDFONT
print "ENDFONT\n";

