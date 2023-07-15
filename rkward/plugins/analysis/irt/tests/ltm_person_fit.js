/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var rad_hypot       = getValue("rad_hypot");
	var rad_resppat     = getValue("rad_resppat");
	var mtx_resppat     = getValue("mtx_resppat");
	var rad_pvalue      = getValue("rad_pvalue");
	var spin_mc         = getValue("spin_mc");

	///////////////////////////////////
	// check for selected options
	var options = new Array() ;
	if (rad_hypot == "greater" || rad_hypot == "two.sided")
		options[options.length] = "alternative=\""+rad_hypot+"\"" ;
	if (rad_resppat == "resp_matrix" && mtx_resppat)
		options[options.length] = "resp.patterns="+mtx_resppat ;
	if (rad_pvalue == "montecarlo")
		options[options.length] = "simulate.p.value=TRUE" ;
	if (rad_pvalue == "montecarlo" && spin_mc != 1000 )
		options[options.length] = "B="+spin_mc ;


	echo ('personfit.res <- person.fit(' + getValue("x"));
		// check if any advanced control options must be inserted
		if (options.length > 0) echo(", "+options.join(", "));
	echo (')\n');
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Person-fit statistics (%1)", getValue("x")) + ')\n');
	}
	echo ('rk.print (' + i18n ("Call:") + ')\n');
	echo ('rk.print.literal (deparse(personfit.res$call, width.cutoff=500))\n');
	echo ('rk.header (' + i18n ("Response patterns, person-fit statistics (L0, Lz) and probabilities for each response pattern (Pr):") + ', level=4)\n');
	echo ('rk.print(cbind(format(personfit.res$resp.patterns, nsmall=0), round(personfit.res$Tobs, digits=3), "Pr (&lt;Lz)"=round(c(personfit.res$p.values), digits=3)))\n');
}
