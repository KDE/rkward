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
	var spin_samples    = getValue("spin_samples");

	echo ('unidim.res <- unidimTest(' + getValue("x"));
		// check if any options must be inserted
		if (spin_samples != 100) echo(", B=" + spin_samples) ;
	echo (')\n');
}

function printout (is_preview) {
	var save         = getValue("save_name.active");
	var save_name    = getValue("save_name");

	if (!is_preview) {
		echo ('rk.header (' + i18n ("Unidimensionality check (%1)", getValue("x")) + ')\n');
	}
	echo ('rk.print (' + i18n ("Call:") + ')\n');
	echo ('rk.print.literal (deparse(unidim.res$call, width.cutoff=500))\n');
	echo ('rk.header (' + i18n ("Matrix of tetrachoric correlations:") + ', level=4)\n');
	echo ('rk.print (unidim.res$Rho)\n');
	echo ('rk.header (' + i18n ("Unidimensionality Check using Modified Parallel Analysis:") + ', level=4)\n');
	echo ('rk.print (' + i18n ("Alternative hypothesis: <em>The second eigenvalue of the observed data is substantially larger than the second eigenvalue of data under the assumed IRT model</em>") + ')\n');
	echo ('rk.print (paste(' + i18n ("Second eigenvalue in the observed data:") + ', round(unidim.res$Tobs[2], digits=3)))\n');
	echo ('rk.print (paste(' + i18n ("Average of second eigenvalues in Monte Carlo samples:") + ', round(mean(unidim.res$T.boot[,2]), digits=3)))\n');
	echo ('rk.print (paste(' + i18n ("Monte Carlo samples:") + ', dim(unidim.res$T.boot)[1]))\n');
	echo ('rk.print (paste(' + i18n ("p-value:") + ', round(unidim.res$p.value, digits=3)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		comment ('keep results in current workspace');
		echo ('.GlobalEnv$' + save_name + ' <- unidim.res\n');
	}
}
