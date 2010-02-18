function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var x = getValue ("xvarslot");
	var y = getValue ("yvarslot");
	if (!(y == "")) {
		y = ", " + y;
	}
	// get additional code (as of now grid) from the calculate section
	var plot_adds = getValue ("plotoptions.code.calculate");

	if (full) {

		echo ('rk.header ("Generic Plot")\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try({\n');
	echo ('	plot(' + x + y + getValue ("plotoptions.code.printout") + ');\n');
	if (plot_adds.length > 0) {
		echo ('\n');
		// print the grid() related code
		printIndented ("\t", plot_adds);
	}

	echo ('})\n');
	if (full) {

		echo ('rk.graph.off ()\n');
	}
}

