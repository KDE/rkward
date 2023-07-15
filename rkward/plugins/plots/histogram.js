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

// internal helper functions
function doPrintout (full) {
	var densityscaled = getValue ("densityscaled");
	var bw =  getValue ("bw");
	var adjust = getValue ("adjust");
	var narm = getValue ("narm");
	var n = getValue ("n"); //called "resolution"
	var x = getValue ("x");

	if (full) {
		echo ('rk.header (' + i18n ("Histogram") + ', list (' + i18n ("Variable") + '=rk.get.description (' + x + ')' + getValue ("histogram_opt.code.preprocess") + '))\n');
		if ((densityscaled) && getValue ("density")) {
			new Header (i18n ("Density curve"), 3).addFromUI ("bw").addFromUI ("adjust").addFromUI ("n").addFromUI ("narm").print ();
		}
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	hist (' + x + getValue ("histogram_opt.code.calculate") + getValue ("histogram_opt.code.printout") + getValue ("plotoptions.code.printout") + ')\n');
	if ((densityscaled) && getValue ("density")) {
		echo ('	lines(density(' + x + ', bw="' + bw + '", adjust = ' + adjust + ', ' + narm + ', n = ' + getValue ("n") + ')' + getValue ("col_density.code.printout") + ')\n');
	}
	
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}


