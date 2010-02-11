function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var varname = getValue ("x");
	var names_mode = getValue ("names_mode");
	var tabulate = getValue ("tabulate");

	var tabulate_header = "";
	if (tabulate) {
		tabulate_header = '"Tabulate", "Yes"';
	} else {
		tabulate_header = '"Tabulate", "No"';
	}

	var barplot_header = getValue ("barplot_embed.code.preprocess");
	var barplot_main = getValue ("barplot_embed.code.printout");


	echo ('x <- ' + varname + "\n");
	if (tabulate) {
		echo ('x <- table(x, exclude=NULL)\n');
	} else {
		echo ('# barplot is a bit picky about attributes, so we need to convert to vector explicitely\n');
		echo ('if(!is.matrix(x)) x <- as.vector(x)\n');
		echo ('if(!is.matrix(x) && is.data.frame(x)) x <- data.matrix(x)\n');
	}

	if (names_mode == "rexp") {
		echo ("names(x) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("names(x) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}

	if (full) {
		echo ('rk.header ("Barplot", parameters=list ("Variable", rk.get.description (' + varname + '), ' + tabulate_header + barplot_header + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndented ("\t", barplot_main);
	echo ('})\n');

	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

