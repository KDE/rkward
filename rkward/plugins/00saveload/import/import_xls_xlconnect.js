/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("convert_encoding.js");

function preprocess (is_preview) {
	if (is_preview) {
		echo ('if (!base::require (XLConnect)) stop (' + i18n ("Preview not available, because package XLConnect is not installed or cannot be loaded.") + ')\n');
	} else {
		echo ('require (XLConnect)\n');
	}
	makeEncodingPreprocessCode ();
}

function preview () {
	preprocess (true);
	calculate (true);
	// no printout()
}

function calculate (is_preview) {
	var options = '';

	var range = getString ("range");
	if (range) options += ', region=' + quote (range);
	else {
		options += makeOption ("startRow", getString ("startrow")) + makeOption ("startCol", getString ("startcol")) + makeOption ("endRow", getString ("endrow")) + makeOption ("endCol", getString ("endcol"));
	}

	if (!getBoolean ("autofitrow")) options += ', autofitRow=FALSE';
	if (!getBoolean ("autofitcol")) options += ', autofitCol=FALSE';
	if (!getBoolean ("header")) options += ', header=FALSE';
	options += makeOption ("rownames", getString ("rownames"));

	if (getValue ("coltypes.columns") > 0) {
		options += ', colTypes=c (' + getList ("coltypes.row.0") + ')';
	}

	echo ('data <- readWorksheetFromFile (' + quote (getString ("file")) + ', sheet=' + getString ("sheet") + options + ')\n');
	makeEncodingCall ('data');
	echo ('\n');
	if (is_preview) {
		echo ('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2]),drop=FALSE]\n');
	} else {
		var object = getString ("saveto");
		echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
		if (getValue ("doedit")) {
			echo ('rk.edit (.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import SPSS data")).addFromUI ("file").addFromUI ("saveto").print ();
}

 
