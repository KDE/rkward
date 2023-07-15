/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var x;
var y;
var mu;
var testForm;
var varequal;
var confint;

function preprocess () {
	x = getValue ("x");
	y = getValue ("y");
	mu = getValue ("mu");
	testForm = getValue ("test_form");

	if (testForm != "const") {
		echo ('names <- rk.get.description (' + x + ", " + y + ')\n');
	} else {
		echo ('names <- rk.get.description (' + x + ')\n');
	}
}

function calculate () {
	varequal = getValue ("varequal");
	confint = getValue ("confint");

	var conflevel = getValue ("conflevel");
	var hypothesis = getValue ("hypothesis");

	var options = ", alternative=\"" + hypothesis + "\"";
	if (testForm == "paired") options += ", paired=TRUE";
	else if (testForm == "indep" && varequal) options += ", var.equal=TRUE";
	if (confint && (conflevel != "0.95")) options += ", conf.level=" + conflevel;

	echo('result <- t.test (x=' + x);
	if(testForm != "const") {
		echo(', y=' + y);
	} else {
		echo(', mu=' + mu);
	}
	echo (options + ')\n');
}

function preview() {
	preprocess();
	calculate();
	printout(true);
}

function printout(is_preview) {
	if (!is_preview) {
		var header = new Header (noquote ('result$method'));
		header.add (i18n ('Comparing'), noquote ('names[1]'));
		header.add (i18nc ("compare against", 'against'), (testForm != "const") ? noquote ('names[2]') : i18n ('constant value: %1', mu));
		header.add ('H1', noquote ('rk.describe.alternative (result)'));
		if (testForm == "indep") {
			header.add (i18n ('Equal variances'), varequal ? i18n ('assumed') : i18n ('not assumed'));
		}
		header.print ();
		echo ('\n');
	}

	echo ('rk.results (list (\n');
	echo ('	' + i18n ('Variable Name') + '=names,\n');
	echo ('	' + i18n ('estimated mean') + '=result$estimate,\n');
	echo ('	' + i18n ('degrees of freedom') + '=result$parameter,\n');
	echo ('	t=result$statistic,\n');
	echo ('	p=result$p.value');
	if (confint) {
		echo (',\n');
		echo ('	' + i18n ('confidence interval percent') + '=(100 * attr(result$conf.int, "conf.level")),\n');
		echo ('	' + i18n ('confidence interval of difference') + '=result$conf.int ');
	}
	echo ('))\n');
}
