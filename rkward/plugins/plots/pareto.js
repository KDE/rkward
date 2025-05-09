/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess() {
	echo('require(qcc)\n');
}

function calculate() {
	if (!getValue("tabulate.checked")) {
		echo('x <- ' + getValue("x") + '\n');
		echo('title <- rk.get.description (' + getValue("x") + ')\n');
		echo('if (!is.numeric (x)) {\n');
		echo('	warning (' + i18n("Data is not numeric, but proceeding as requested.\\nDid you forget to check the tabulate option?") + ')\n');
		echo('}\n');
	} else {
		echo(getValue('tabulate_options.code.calculate'));
	}

	if (getValue("limit.checked")) {
		echo(getValue('limit_options.code.calculate'));
	}
}

function printout() {
	doPrintout(true);
}

function preview() {
	preprocess();
	calculate();
	doPrintout(false);
}

function doPrintout(full) {
	var descriptives = getValue("descriptives") == "TRUE";

	if (full) {
		echo('rk.header (' + i18n("Pareto chart") + ', parameters=list (');
		if (getValue("tabulate.checked")) echo(getValue('tabulate_options.parameters'));
		else echo(i18n("Variable") + '=title');
		if (getValue("limit.checked")) echo(', ' + getValue('limit_options.parameters'));
		echo('))\n');
		echo('\n');

		echo('rk.graph.on ()\n');
	}

	echo('try ({\n');
	printIndentedUnlessEmpty("\t", getValue("plotoptions.code.preprocess"), '', '\n');

	echo('\t');
	if (full && descriptives) echo('descriptives <- ');
	echo('pareto.chart(x' + getValue("plotoptions.code.printout") + ')\n');
	if (full && descriptives) echo('	rk.results(descriptives, titles=c(NA,NA))\n');

	printIndentedUnlessEmpty("\t", getValue("plotoptions.code.calculate"), '\n', '');
	echo('})\n');
	if (full) {
		echo('rk.graph.off ()\n');
	}
}
