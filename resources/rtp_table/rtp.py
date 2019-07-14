#!/usr/bin/env python3
# Usage example:
# ./rtp.py 0 < RTP2k.csv > rtp.cpp
# ./rtp.py 1 < RTP2k3.csv >> rtp.cpp

from sys import stdin, argv

if len(argv) < 2:
	raise ValueError("Specify 0 (2k) or 1 (2k3)")

rtp_table = "3" if argv[1] == "1" else ""

firstline = stdin.readline()
elems = len(firstline.split(","))
first = ""
lines = []
lookup = {}

for i, line in enumerate(stdin):
	l = line.lower().strip().split(",")
	lst = [""]*elems
	if len(l[0]) > 0:
		first = l[0]
		lookup[first] = i
	lst[0] = first
	lst[1:] = l[1:]
	while len(lst) < elems:
		lst.append("")
	lines.append(lst)

print("const char* const rtp_table_2k%s[][%s] = {" % (rtp_table, str(elems)))
for l in lines:
	print("\t{", end="")
	for i in range(elems):
		ll = l[i]
		if len(ll) == 0:
			if i == (elems - 1):
				print("nullptr", end="")
			else:
				print("nullptr, ", end="")
		elif i == (elems - 1):
			s = '"' + ll + '"'
			print(s, end="")
		else:
			s = '"' + ll + '"'
			print(s + ', ', end="")

	print("},")

print("	{", end="")
for i in range(elems):
	if i == elems - 1:
		print("nullptr", end="")
	else:
		print("nullptr, ", end="")
print("}")
print("};")

print("");
print("const char* const rtp_table_2k%s_categories[%s] = {" % (rtp_table, len(lookup.items()) + 1))

for k in lookup.keys():
	print('\t"%s",' % k)

print("\tnullptr")
print("};")

print("")
print("const int rtp_table_2k%s_categories_idx[%s] = {" % (rtp_table, len(lookup.items()) + 1))

for v in lookup.values():
	print('\t%s,' % v)

print("\t%s" % len(lines))
print("};")
print("")
