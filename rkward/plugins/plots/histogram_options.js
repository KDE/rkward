/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var histcalcoptions;
var histplotoptions;
var headeroptions;

function makeCodes () {
	histcalcoptions = ", breaks=";
	histplotoptions = "";
	var varname = getValue ("varname");

	var histbreaks = getValue ("histbreaksFunction");
	var header = new Header ();
	var bp = i18n ("Break points");
	if (histbreaks == "cells") {
		histcalcoptions += getValue ("histbreaks_ncells");
		header.add (bp, i18n ("Approximately %1 cells", getValue ("histbreaks_ncells")));
	} else if (histbreaks == "int") {
		histcalcoptions += "seq (floor (min (" + varname + ", na.rm=TRUE))-0.5, ceiling (max (" + varname + ", na.rm=TRUE))+0.5)";
		header.add (bp, i18n ("Integers"));
	} else if (histbreaks == "vec") {
		histcalcoptions += "(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=" + getValue ("histbreaks_veclength") + ")})(" + varname + ")";
		header.add (bp, i18n ("Equally spaced vector of length %1", getValue ("histbreaks_veclength")));
	} else {
		histcalcoptions += "\"" + histbreaks + "\"";
		header.add (bp, histbreaks);
	}

	if (!getBoolean ("rightclosed")) histcalcoptions += ", right=FALSE";
	header.addFromUI ("rightclosed");
	if (!getBoolean ("include_lowest")) histcalcoptions += ", include.lowest=FALSE";
	header.addFromUI ("include_lowest");

	if (!getBoolean ("freq")) {
		histplotoptions += ", freq=FALSE";
		header.add (i18n ("Scale"), i18n ("Density"));
	} else {
		header.add (i18n ("Scale"), i18n ("Frequency"));
	}

	var addbars = getValue ("addtoplot");
	if (addbars) histplotoptions += ", add=TRUE";

	var labels = getValue ("barlabels");
	if (labels) histplotoptions += ", labels=TRUE";

	var histlty = getValue ("histlinetype");
	histplotoptions += ", lty=" + "\"" + histlty + "\"";

	var histbordercol = "";
	if (histlty != "blank") {
		var density = getValue ("density");
		histplotoptions += ", density=" + density;
		if (density > 0) histplotoptions += ", angle=" + getValue ("angle");
		if (getValue ("doborder")) {
			histbordercol = getValue ("histbordercol.code.printout");
		} else {
			histbordercol = ", border=FALSE";
		}
	}

	var histfillcol = "";
	if (getValue ("usefillcol")) histfillcol = getValue ("histfillcol.code.printout");

	histplotoptions += histbordercol + histfillcol;
	headeroptions = ", " + header.extractParameters ();
}

function preprocess () {
	makeCodes();

	echo (headeroptions);
}

function calculate () {
	// makeCodes() has already run

	echo (histcalcoptions);
}

function printout () {
	// makeCodes() has already run

	echo (histplotoptions);
}

