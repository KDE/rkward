#!/usr/bin/python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: 2023 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

# crude helper script to add one of the more common REUSE copyright headers, to files where it is still missing
# please check results, manually

# REUSE-IgnoreStart

import os
import sys

rootdir = sys.argv[1]

def rewrite (filename):
        with open(filename, 'r') as src:
            content = src.read()
        if (content.find("SPDX-FileCopyrightText") > -1):
            print(filename + " has header")
            return

        holdern = input(filename + " Prim copyright holder: [0] skip [1] Thomas [2] Meik [3] Prasenjit [4] Stefan? ")
        holder = ""
        if (holdern.startswith("1")):
            holder = "Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>"
        elif (holdern.startswith("2")):
            holder = "Meik Michalke <meik.michalke@hhu.de>"
        elif (holdern.startswith("3")):
            holder = "Prasenjit Kapat <rkward@kde.org>"
        elif (holdern.startswith("4")):
            holder = "Stefan Rödiger <stefan_roediger@gmx.de>"
        else:
            print("skipping")
            return
        
        text = "- This file is part of the RKWard project (https://rkward.kde.org).\n"
        text += "SPDX-FileCopyrightText: by " + holder + "\n"
        text += "SPDX-FileContributor: The RKWard Team <rkward@kde.org>\n"
        text += "SPDX-License-Identifier: GPL-2.0-or-later"

        if (content.startswith("<!DOCTYPE") or filename.endswith(".xml")):
            with open(filename, 'wt') as dst:
                index = content.find("\n") + 1
                dst.write(content[:index])
                dst.write("<!--")
                dst.write(text)
                dst.write("\n-->\n")
                dst.write(content[index:])
        elif (filename.endswith(".js")):
            with open(filename, 'wt') as dst:
                dst.write("/*")
                dst.write(text)
                dst.write("\n*/\n")
                dst.write(content)
        elif (filename.endswith(".R") or filename.endswith("CMakeLists.txt")):
            with open(filename, 'wt') as dst:
                dst.write("# " + text.replace("\n", "\n# ") + "\n")
                dst.write(content)
        elif (filename.endswith(".sh") or filename.endswith(".py")):
            index = 0;
            if (content.startswith("#!")):
                index = content.find("\n")+ 1
            with open(filename, 'wt') as dst:
                dst.write(content[:index])
                dst.write("# " + text.replace("\n", "\n# ") + "\n")
                dst.write(content[index:])
        else:
            print(filename + " not handled")

for folder, subs, files in os.walk(rootdir):
    for filename in files:
        if filename.startswith("po"):
            continue
        if (True or filename.endswith(".xml")):
            rewrite(os.path.join(folder, filename))

# REUSE-IgnoreEnd
