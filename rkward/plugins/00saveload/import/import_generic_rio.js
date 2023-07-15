/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include("convert_encoding.js");

function preprocess() {
	echo('require(rio)\n');
	makeEncodingPreprocessCode();
}

function preview() {
	preprocess();
	calculate(true);
}

function calculate(is_preview) {
	var which = getString("which");
	if (which != "") which = ', which=' + which;
	var import_list = getBoolean("import_list");
	var object = getValue("saveto");

	if (import_list) {
		echo('data <- import_list("' + getValue("file") + '")\n');
	} else {
		echo('data <- import("' + getValue("file") + '"' + which + ')\n');
	}

	makeEncodingCall('data');
	if (is_preview) {
		if (import_list) {
			echo('data <- data[[1]]');
		}
		echo('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2]),drop=FALSE]\n');
	} else {
		echo('.GlobalEnv$' + object + ' <- data  '); comment('assign to globalenv()');
		if (getValue("doedit")) {
			echo('rk.edit(.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Generic data import")).addFromUI ("file").addFromUI ("saveto").print ();
}

