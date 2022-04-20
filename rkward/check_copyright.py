#!/usr/bin/python
import sys
import re

print("Processing " + sys.argv[1])

f = open(sys.argv[1], "r")
lines = f.readlines()

line_num = 0
fname = ""
begin = ""
copyright = ""
gplfound = False
delims_found = 0

def die(message):
	print("error " + message + " in file " + sys.argv[1])
	exit()

for line in lines:
	if (line_num > 20):
		die("too long")
	line = line.strip()
	if (line.__contains__("description")):
		fname = re.split(r"\s+", line)[0]
	elif (line.startswith("begin")):
		begin = re.split(r":", line, maxsplit=1)[1].strip()
	elif (line.startswith("copyright")):
		copyright = re.split(r":", line)[1].strip()
	elif (line.__contains__("the Free Software Foundation; either version 2 of the License, or")):
		gplfound = True
	elif line.endswith("*************************/"):
		delims_found += 1
		if (delims_found == 2):
			break
	line_num += 1

begin = re.sub(r"\d\d:\d\d:\d\d\s[A-Z][A-Z][A-Z]\s", "", begin)
copyright = re.sub(r"(C) ", "", copyright)
cyears, cholder = re.split(r" by ", copyright, maxsplit=1)
years = re.findall(r"\d\d\d\d", cyears)
year1 = years[0]
year2 = years[len(years)-1]
if (year1 != year2):
	years = year1 + "-" + year2
else:
	years = year1

cholder = cholder.strip()
if cholder != "Thomas Friedrichsmeier":
	die("Notme")
if fname == "":
	die("empty")
if not gplfound:
	die("nogpl")

o = open(sys.argv[1], "w")
o.write("/*\n")
o.write(fname + " - This file is part of the RKWard project. Created: " + begin + "\n")
o.write("SPDX-FileCopyrightText: " + years + " by " + cholder + " <thomas.friedrichsmeier@kdemail.net>\n")
o.write("SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>\n")
o.write("SPDX-License-Identifier: GPL-2.0-or-later\n")
o.write("*/\n")
lines = lines[line_num+1 : ]
for line in lines:
	o.write(line)
	if not line.endswith("\n"):
		o.write("\n")
o.close()
