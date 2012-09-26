function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getValue ("x");

	var tabulate = getValue ("tabulate.checked");
	var main_header = '"Variable"=rk.get.description (' + vars + ')';
	if (tabulate) main_header = getValue ('tabulate_options.parameters');

	var limit = getValue ("limit.checked");
	var limit_header = "";
	if (limit) limit_header = ", " + getValue ('limit_options.parameters');

	var radius = getValue ("radius");
	var angle = getValue ("angle");
	var angle_inc = getValue ("angle_inc");
	var density = getValue ("density");
	var density_inc = getValue ("density_inc");
	var col = getValue ("colors");
	var clockwise = getValue ("clockwise");
	var clockwise_header = "";
	if (clockwise) {
		clockwise_header = ', "Orientation"="Clockwise"';
	} else {
		clockwise_header = ', "Orientation"="Counter clockwise"';
	}
	var names_mode = getValue ("names_mode");

	var options = ", clockwise =" + clockwise;
	if ((density >= 0) || (density_inc != 0)) options += ", density =" + density;
	if (density_inc != 0) options += " + " + density_inc + " * 0:length (x)";
	if ((density > 0) || density_inc != 0) {
		options += ", angle =" + angle;
		if (angle_inc != 0) options += " + " + angle_inc + " * 0:length (x)";
	}
	if (radius != 0.8) options += ", radius=" + radius;
	if (col == "rainbow") options += ", col=rainbow (if(is.matrix(x)) dim(x) else length(x))";
	else if (col == "grayscale") options += ", col=gray.colors (if(is.matrix(x)) dim(x) else length(x))";
	options += getValue ("plotoptions.code.printout");

	if (tabulate) {
		echo (getValue ('tabulate_options.code.calculate'));
	} else {
		echo ('x <- ' + getValue ("x") + '\n');
		echo ('if (!is.numeric (x)) {\n');
		echo ('	warning ("Data is not numeric, but proceeding as requested.\\nDid you forget to check the tabulate option?")\n');
		echo ('}\n');
	}

	if (getValue ("limit.checked")) {
		echo (getValue ('limit_options.code.calculate'));
	}
	echo ('\n');

	if (full) {
		echo ('rk.header ("Pie chart", parameters=list (' + main_header + limit_header + clockwise_header + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');
	if (names_mode == "rexp") {
		echo ("\tnames(x) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("\tnames(x) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}

	echo ('	pie(x' + options + ')\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

