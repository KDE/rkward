This directory contains files to create a windows installer based on the
Nullsoft Scriptable Install System (NSIS): http://nsis.sourceforge.net

Use make_release.bat to create a windows release from the current SVN-state.
The first few lines of that file may need customization.

Just like the debian/ directory for debian packaging, this directory is not
meant to become part of source releases.

During creation of the installer, a number of files are automatically generated
inside this directory. Please don't add those to SVN.
