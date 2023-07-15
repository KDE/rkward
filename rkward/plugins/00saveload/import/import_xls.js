/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preview () {
	preprocess (true);
	calculate (true);
	// printout ();
}

function preprocess (is_preview) {
	if (is_preview) {
		echo ('if (!base::require (gdata)) stop (' + i18n ("Preview not available, because package gdata is not installed or cannot be loaded.") + ')\n');
	} else {
		echo ('require (gdata)\n');
	}
}

function calculate (is_preview) {
	var header = getValue ("header");
	var verbose = getValue ("verbose");
	var sheet = getValue ("sheetname");

	var quote_char = getValue ("quote");
	if (quote_char == "other") quote_char = quote (getValue ("custom_quote"));

	var options = ", header=" + header + makeOption ("quote", quote_char) + ", verbose=" + verbose;

	echo ('data <- read.xls ("' + getValue ("file") + '", sheet="' + sheet + '"' + options + ', ');
	echo (' nrows=' + getValue ("nrows") + ', skip=' + getValue ("skip") + ', na.string="'+ getValue ("na") +'"' + getValue("strings_as_factors") + 
	      ', strip.white = ' + getValue("stripwhite") + ')\n');

	if (is_preview) {
		echo ('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2]),drop=FALSE]\n');
	} else {
		var object = getValue ("saveto");
		echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
		if (getValue ("doedit") ) {
			echo ('rk.edit (.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import Microsoft EXCEL sheet"))
		.addFromUI ("file")
		.addFromUI ("saveto")
		.addFromUI ("sheetname")
		.addFromUI ("header")
		.addFromUI ("skip")
		.addFromUI ("nrows")
		.addFromUI ("na").print ();
}

