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

function doPrintout (full) {
	var vars = getValue ("x");
	var names_mode = getValue ("names_mode");

	var tabulate = getValue ("tabulate.checked");
	var main_header = i18n ("Variable") + '=rk.get.description (' + vars + ')';
	if (tabulate) main_header = getValue ('tabulate_options.parameters');

	var limit = getValue ("limit.checked");
	var limit_header = "";
	if (limit) limit_header = ", " + getValue ('limit_options.parameters');

	var options = getValue ("plotoptions.code.printout");

	if (tabulate) {
		echo (getValue ('tabulate_options.code.calculate'));
		echo ('n <- names (x); x <- as.numeric (x); names (x) <- n		'); comment ('dotchart() is somewhat picky about data type');
	} else {
		echo ('x <- ' + getValue ("x") + '\n');
		echo ('if (!is.numeric (x)) {\n');
		echo ('	warning (' + i18n ("Data is not numeric, but proceeding as requested.\\nDid you forget to check the tabulate option?") + ')\n');
		echo ('}\n');
	}

	if (getValue ("limit.checked")) {
		echo (getValue ('limit_options.code.calculate'));
	}
	echo ('\n');

	if (full) {
		echo ('rk.header (' + i18n ("Dot chart") + ', parameters=list (' + main_header + limit_header + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	if (names_mode == "rexp") {
		echo ("names(x) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("names(x) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');
	echo ('	dotchart(x' + options + ')\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

