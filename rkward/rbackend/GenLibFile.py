# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: LGPL-2.1-or-later
#
# Generate a .lib file for a given .dll
# This assumed dumpbin and lib to be in the path, which they should be, when compiling with MSVC (and that's what this is needed for)
#
# Usage: python3 GenLibFile.py XYZ.dll output_directory architecture

import subprocess
import os
import sys

dllfile = sys.argv[1]
workdir = sys.argv[2]
arch = sys.argv[3]
base = os.path.basename(dllfile).replace(".dll", "")
deffile = base + ".def"
libfile = base + ".lib"

dump = subprocess.check_output(["dumpbin", "/exports", dllfile]).decode("latin1").splitlines()
exports = []
for line in dump:
    fields = line.split()
    if len(fields) != 4:
        continue
    exports.append(fields[3])
os.chdir(workdir)
with open(os.path.join(workdir, deffile), "wt+") as outdef:
    outdef.write("EXPORTS\n")
    outdef.write("\n".join(exports))
subprocess.call(["lib", f"/def:{deffile}", f"/out:{libfile}", f"/machine:{arch}"])
