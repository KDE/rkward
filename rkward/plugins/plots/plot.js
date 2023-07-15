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
	var x = getValue ("xvarslot");
	var y = getValue ("yvarslot");
	if (!(y == "")) {
		y = ", " + y;
	}

	if (full) {
		new Header (i18n ("Generic Plot")).print ();
		echo ('rk.graph.on ()\n');
	}

	echo ('try({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	plot(' + x + y + getValue ("plotoptions.code.printout") + ');\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {

		echo ('rk.graph.off ()\n');
	}
}

