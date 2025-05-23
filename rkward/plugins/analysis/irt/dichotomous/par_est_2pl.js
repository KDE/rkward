/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess() {
	// we'll need the ltm package, so in case it's not loaded...
	echo('require(ltm)\n');
}

function calculate() {
	// let's read all values into variables for the sake of readable code
	var constraint = getValue("constraint");
	var startval = getValue("startval");
	var startval_mtx = getValue("startval_mtx");
	var naaction = getValue("naaction");
	var irtparam = getValue("irtparam");
	var optimeth = getValue("optimeth");
	var verbose = getValue("verbose");
	// these are 2pl specific
	var ghk_2pl = getValue("ghk_2pl");
	var iterem = getValue("iterem");
	var iterqn_2pl = getValue("iterqn_2pl");
	var interact = getValue("interact");

	///////////////////////////////////
	// check for selected advanced control options
	var control = new Array();
	if (iterem != "40")
		control[control.length] = "iter.em=" + iterem;
	if (iterqn_2pl != "150")
		control[control.length] = "iter.qN=" + iterqn_2pl;
	if (ghk_2pl != "15")
		control[control.length] = "GHk=" + ghk_2pl;
	if (optimeth != "BFGS")
		control[control.length] = "method=\"" + optimeth + "\"";
	if (verbose == "TRUE")
		control[control.length] = "verbose=TRUE";

	echo('estimates.2pl <- ltm(' + getValue("x") + ' ~ z1');
	// any additional options?
	if (interact == "TRUE") echo(" * z2");
	if (constraint) echo(", constraint=" + constraint);
	if (irtparam != "TRUE") echo(", IRT.param=FALSE");
	if (startval == "random") echo(", start.val=\"random\"");
	if (startval == "matrix") echo(", start.val=" + startval_mtx);
	if (naaction) echo(", na.action=" + naaction);
	// finally check if any advanced control options must be inserted
	if (control.length > 0) echo(", control=list(" + control.join(", ") + ")");
	echo(')\n');
}

function printout(is_preview) {
	// check whether parameter estimations should be kept in the global enviroment
	var save = getValue("save_name.active");
	var save_name = getValue("save_name");

	if (!is_preview) {
		echo('rk.header (' + i18n("2PL parameter estimation") + ')\n');
	}
	echo('rk.print (' + i18n("Call:") + ')\n');
	echo('rk.print.literal (deparse(estimates.2pl$call, width.cutoff=500))\n');
	echo('rk.header (' + i18n("Coefficients:") + ', level=4)\n');
	echo('rk.print (coef(estimates.2pl))\n');
	echo('rk.print (paste(' + i18n("Log-likelihood value at convergence:") + ',round(estimates.2pl$log.Lik, digits=1)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		comment('keep results in current workspace');
		echo('.GlobalEnv$' + save_name + ' <- estimates.2pl\n');
	}
}
