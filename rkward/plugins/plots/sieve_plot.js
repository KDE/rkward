/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(vcd)\n');
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getValue ("x");
	var shade = getValue ("shade");
	var sievetype = getValue ("sievetype");

	echo ('x <- ' + vars + '\n');
	if (full) {
		new Header (i18n ("Extended Sieve Plot")).add (i18n ("Variable"), noquote ('rk.get.description (' + vars + ')')).addFromUI ("shade").print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	sieve(x, shade = ' + shade + ', sievetype = "' + sievetype + '" ' + getValue ("plotoptions.code.printout") + ')\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}
