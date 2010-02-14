function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getValue ("x");
	var names_mode = getValue ("names_mode");
	var tabulate = getValue ("tabulate");

	var tabulate_header = "";
	if (tabulate) {
		tabulate_header = '"Tabulate", "Yes"';
	} else {
		tabulate_header = '"Tabulate", "No"';
	}

	var options = getValue ("plotoptions.code.printout");

	var plotpre = getValue ("plotoptions.code.preprocess");
	var plotpost = getValue ("plotoptions.code.calculate");

	if (tabulate) {
		echo ('x <- table (' + vars + ', exclude=NULL)\n');
	} else {
		echo ('x <- ' + vars);
		echo ('\n');
		echo ('if (!is.numeric (x)) {\n');
		echo ('	warning ("Data may not be numeric, but proceeding as requested.\\nDid you forget to check the tabulate option?")\n');
		echo ('}\n');
	}
	echo ('\n');
	if (full) {
		echo ('rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (' + vars + '), ' + tabulate_header + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	if (names_mode == "rexp") {
		echo ("names(x) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("names(x) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}
	if (plotpre != "") printIndented ("\t", plotpre);
	echo ('	dotchart(x' + options + ')\n');
	if (plotpost != "") printIndented ("\t", plotpost);
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

