function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getValue ("x");
	var tabulate = getValue ("tabulate");
	var tabulate_header = "";
	if (tabulate) {
		tabulate_header = '"Tabulate", "Yes"';
	} else {
		tabulate_header = '"Tabulate", "No"';
	}
	var radius = getValue ("radius");
	var angle = getValue ("angle");
	var angle_inc = getValue ("angle_inc");
	var density = getValue ("density");
	var density_inc = getValue ("density_inc");
	var col = getValue ("colors");
	var clockwise = getValue ("clockwise");
	var clockwise_header = "";
	if (clockwise) {
		clockwise_header = '"Clockwise", "Yes"';
	} else {
		clockwise_header = '"Clockwise", "No"';
	}
	var names_mode = getValue ("names_mode");

	var options = ", clockwise =" + clockwise;
	if ((density >= 0) || (density_inc != 0)) options += ", density =" + density;
	if (density_inc != 0) options += " + " + density_inc + " * 0:length (x)";
	if ((density > 0) || density_inc != 0) {
		options += ", angle =" + angle;
		if (angle_inc != 0) options += " + " + angle_inc + " * 0:length (x)";
	}
	if (radius != 0+8) options += ", radius=" + radius;
	if (col == "rainbow") options += ", col=rainbow (if(is.matrix(x)) dim(x) else length(x))";
	else if (col == "grayscale") options += ", col=gray.colors (if(is.matrix(x)) dim(x) else length(x))";
	options += getValue ("plotoptions.code.printout");

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
		echo ('rk.header ("Pie chart", parameters=list ("Variable", rk.get.description (' + vars + '), ' + tabulate_header + ', ' + clockwise_header + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	if (plotpre.length > 0) printIndented ("\t", plotpre);
	if (names_mode == "rexp") {
		echo ("\tnames(x) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("\tnames(x) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}

	echo ('	pie(x' + options + ')\n');
	if (plotpost.length > 0) printIndented ("\t", plotpost);
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

