/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
var dodensity_plot;

function preprocess () {
	dodensity_plot = (getValue ("plot_type") == "density_plot");

	if (!dodensity_plot) {
		echo ('require(hdrcde)\n');
	}
}


function preview () {
	preprocess ();
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout (full) {
	var adjust = getValue ("adjust");
	var x = getValue ("x");
	var resolution = getValue ("n");
	var narm = getValue ("narm");
	var kern = getValue ("kern");
	
	var bw = "";
	if (kern == "gaussian") {
		bw = getValue ("bw");
	}
	var dorug = getValue ("rug");

	var density_call = "density(" + x;
	if (bw != "") density_call += ", bw=\"" + bw + "\"";
	density_call += ", adjust=" + adjust + ", kern=\"" + kern + "\", n=" + resolution + ", " + narm + ")";

	var title = "";
	if (dodensity_plot) title = i18n ("Density Plot");
	else title = i18n ("Highest density regions");

	if (full) {
		header = new Header (title);
		header.add (i18n ("Variable"), noquote ('rk.get.description (' + x + ')'));
		header.add (i18n ("Length"), noquote ('length (' + x + ')'));
		header.addFromUI ("adjust");
		header.addFromUI ("narm");
		header.addFromUI ("n");
		header.addFromUI ("kern");
		if (bw != "") header.addFromUI ("bw");
		header.print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	if (dodensity_plot) {
		echo ('	plot(' + density_call + getValue ("plotoptions.code.printout") + ')\n');
	} else {
		echo ('	hdr.den(den=' + density_call + getValue ("plotoptions.code.printout") + ')\n');
	}
	if (dorug) {
		echo ('	rug(' + x + ', ' + getValue ("rug_ticksize") + ', ' + getValue ("rug_lwd") + ', ' + getValue ("rug_side") + getValue ("rug_col.code.printout") + ')\n');
	}

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

