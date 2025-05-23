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
	// let's read all values into php variables for the sake of readable code
	var constraint = getValue("constraint");
	var startval = getValue("startval");
	var startval_lst = getValue("startval_lst");
	var hessian = getValue("hessian");
	var naaction = getValue("naaction");
	var irtparam = getValue("irtparam");
	var optimeth = getValue("optimeth");
	var verbose = getValue("verbose");
	// these are grm specific
	var ghk_grm = getValue("ghk_grm");
	var iterqn_grm = getValue("iterqn_grm");
	var dig_abbrv = getValue("dig_abbrv");

	///////////////////////////////////
	// check for selected advanced control options
	var control = new Array();
	if (iterqn_grm != 150)
		control[control.length] = "iter.qN=" + iterqn_grm;
	if (ghk_grm != 21)
		control[control.length] = "GHk=" + ghk_grm;
	if (optimeth != "BFGS")
		control[control.length] = "method=\"" + optimeth + "\"";
	if (verbose == "TRUE")
		control[control.length] = "verbose=TRUE";
	if (dig_abbrv != 6)
		control[control.length] = "digits.abbrv=" + dig_abbrv;

	echo('estimates.grm <- grm(' + getValue("x"));
	// any additional options?
	if (constraint == "const_discr") echo(", constrained=TRUE");
	if (irtparam != "TRUE") echo(", IRT.param=FALSE");
	if (hessian == "hessian") echo(", Hessian=TRUE");
	if (startval == "random") echo(", start.val=\"random\"");
	if (startval == "list") echo(", start.val=" + startval_lst);
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
		echo('rk.header (' + i18n("GRM parameter estimation") + ')\n');
	}
	echo('rk.print (' + i18n("Call:") + ')\n');
	echo('rk.print.literal (deparse(estimates.grm$call, width.cutoff=500))\n');
	echo('rk.header (' + i18n("Coefficients:") + ', level=4)\n');
	echo('rk.print (coef(estimates.grm))\n');
	echo('rk.print (paste(' + i18n("Log-likelihood value at convergence:") + ',round(estimates.grm$log.Lik, digits=1)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		comment('keep results in current workspace');
		echo('.GlobalEnv$' + save_name + ' <- estimates.grm\n');
	}
}
