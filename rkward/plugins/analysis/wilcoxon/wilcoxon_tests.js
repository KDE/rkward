/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess(){
	var x = getValue("x");
	var y = getValue("y");
	var ties = getValue("ties");
	if(ties) {
		echo("\trequire(exactRankTests)\n");
	} else {}
	echo("\tnames <- rk.get.description (" + x);
	if(y) {
		echo(", " + y);
	} else {}
	echo(")\n");
}

function calculate(){
	// read in variables from dialog
	var x = getValue("x");
	var y = getValue("y");
	var alternative = getValue("alternative");
	var paired = getValue("paired");
	var svbSvrsltst = getValue("svb_Svrsltst");
	var conflevel = getValue("conflevel");
	var mu = getValue("mu");
	var exact = getValue("exact");
	var correct = getValue("correct");
	var ties = getValue("ties");
	var confintChecked = getValue("confint.checked");

	// the R code to be evaluated
	var confintChecked = getValue("confint.checked");
	echo("\twcox.result <- ");
	if(ties) {
		echo("wilcox.exact(");
	} else {
		echo("wilcox.test(");
	}
	if(x) {
		echo("\n\t\tx=" + x);
	} else {}
	if(y) {
		echo(",\n\t\ty=" + y);
	} else {}
	if(alternative != "two.sided") {
		echo(",\n\t\talternative=\"" + alternative + "\"");
	} else {}
	if(mu != 0) {
		echo(",\n\t\tmu=" + mu);
	} else {}
	if(y && paired) {
		echo(",\n\t\tpaired=TRUE");
	} else {}
	if(exact != "automatic") {
		echo(",\n\t\texact=" + exact);
	} else {}
	if(correct) {
		echo(",\n\t\tcorrect=TRUE");
	} else {}
	if(confintChecked) {
		echo(",\n\t\tconf.int=TRUE");
		if(conflevel != 0.95) {
			echo(",\n\t\tconf.level=" + conflevel);
		} else {}
	} else {}
	echo("\n\t)\n\n");
}

function printout(is_preview){
	// printout the results

	var y = getValue("y");
	var confintChecked = getValue("confint.checked");
	var correct = getValue("correct");
	var mu = getValue("mu");
	if (!is_preview) {
		var header = new Header (noquote ('wcox.result$method'));
		header.add (i18n ("Comparing"), noquote ('names[1]'));
		if (y) header.add (i18nc ("compare against", "against"), noquote ('names[2]'));
		header.add ("H1", noquote ('rk.describe.alternative (wcox.result)'));
		header.add (i18n ("Continuity correction in normal approximation for p-value"), correct ? "TRUE" : "FALSE");
		header.addFromUI ("exact");
		if (y) header.addFromUI ("paired");
		header.addFromUI ("mu");
		header.print ();
	}
	echo("rk.results (list (\n\t" + i18n ("Variable Names") + "=names,\n\t" + i18nc ("a statistic indicator", "Statistic") + "=wcox.result$statistic,\n\t" + i18n ("Location Shift") + "=wcox.result$null.value,\n\t" + i18n ("Hypothesis") + "=wcox.result$alternative,\n" + "\tp=wcox.result$p.value");
	if(confintChecked) {
		echo(",\n\t" + i18n ("Confidence interval percent") + "=(100 * attr(wcox.result$conf.int, \"conf.level\")),\n\t" + i18n ("Confidence interval of difference") + "=wcox.result$conf.int,\n\t" + i18n ("Difference in Location") + "=wcox.result$estimate");
	} else {}
	echo("))\n");
	// save result, if requested
	if(getBoolean("svb_Svrsltst.active")) {
		echo("\n\t.GlobalEnv$" + getString("svb_Svrsltst") + " <- wcox.result\n");
	}
}
