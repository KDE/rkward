/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var opts = "";
	var x = getValue ("x");
	var g = getValue ("g");
	var method = '"' + getValue ("method") + '"';
	if (method == "\"jitter\"") {
		opts += ", jitter = " + getValue ("jitter");
	} else if (method == "\"stack\"") {
		opts += ", offset = " + getValue ("offset");
	}
	if (getValue ("orientation") == "Vertical") opts += ", vertical = TRUE";

	if (full) {
		header = new Header (i18n ("Stripchart"))
			.add (i18n ("Variable"), noquote ('rk.get.description (' + x + ')'))
			.add (i18n ("Group"), noquote ('rk.get.description (' + g + ')'))
			.addFromUI ("method");
		if (method == "\"jitter\"") header.addFromUI ("jitter");
		else if (method == "\"stack\"") header.addFromUI ("offset");
		header.addFromUI ("orientation");
		header.print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('\tstripchart (' + x + ' ~ (' + g + '), method = ' + method + opts + getValue ("plotoptions.code.printout") + ')\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');

	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

