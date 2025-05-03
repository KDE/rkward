/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!
//
// look for a file called: $SRC/inst/rkward/rkwarddev_rk.download_appimage_plugin_script.R

function preprocess(is_preview) {
	// add requirements etc. here
	var noLoadMsg = getValue("noLoadMsg");
	if (noLoadMsg) {
		echo("suppressMessages(require(XiMpLe))\n");
	} else {
		echo("require(XiMpLe)\n");
	}
}

function calculate(is_preview) {
	// read in variables from dialog
	var aiuFile = getString("aiu_file");
	var aiuBranch = getString("aiu_branch");
	var aiuUrl = getString("aiu_url");
	var aiuPattern = getString("aiu_pattern");
	var aiuMethod = getString("aiu_method");
	var aiuTimeout = getString("aiu_timeout");
	var aiuCacheok = getBoolean("aiu_cacheok.state");

	// the R code to be evaluated
	var aiuFileOverwrite = getValue("aiu_file.overwrite");
	var fileBasename = aiuFile.replace(/.*\//, '');
	var fileDirname = aiuFile.match(/.*\//);
	echo("appimage <- rk.with.progress(\n  {rk.download_appimage(\n" +
	     "    dir = \"" + fileDirname + "\"" +
	     ",\n    filename = \"" + fileBasename + "\"");
	if (aiuFileOverwrite) {
		echo(",\n    overwrite = TRUE");
	} else {
		echo(",\n    overwrite = FALSE");
	}
	if (aiuBranch == "stable") {
		echo(",\n    url = \"https://download.kde.org/stable/rkward/0.8.1\"" +
		     ",\n    pattern = \"rkward.*x86_64\\\\.AppImage\"");
	} else if (aiuBranch == "develop") {
		echo(",\n    url = \"https://cdn.kde.org/ci-builds/education/rkward/master/linux\"" +
		     ",\n    pattern = \"rkward-master.*linux-gcc-x86_64\\\\.AppImage\"");
	} else {
		echo(",\n    url = \"" + aiuUrl + "\"" +
		     ",\n    pattern = \"" + aiuPattern + "\"");
	}
	echo(",\n    method = \"" + aiuMethod + "\"");
	if (aiuCacheok) {
		echo(",\n      cacheOK=TRUE");
	} else {
	}
	echo(",\n    timeout = " + aiuTimeout + "\n  )},\n  text = \"Downloading RKWard AppImage...\"\n)\n\n");
}

function printout(is_preview) {
	// printout the results
	new Header(i18n("Download AppImage results")).print();
	echo("rk.print.literal(\"AppImage was saved as:\")\n" +
	     "rk.results(appimage)\n" +
	     "rk.print.literal(\"You must restart RKWard to use it.\")\n");
}
