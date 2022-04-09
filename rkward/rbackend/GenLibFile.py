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
