function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

// internal helper functions
function doPrintout (full) {
	var densityscaled = getValue ("densityscaled");
	var bw =  getValue ("bw");
	var adjust = getValue ("adjust");
	var narm = getValue ("narm");
	var n = getValue ("n"); //called "resolution"
	var x = getValue ("x");
	var plot_adds = getValue ("plotoptions.code.calculate");

	if (full) {
		echo ('rk.header ("Histogram", list ("Variable", rk.get.description (' + x + ') ');
		if ((densityscaled) && getValue ("density")) {
			echo (', "Density bandwidth", "');
			echo (bw);
			echo ('", "Density adjust", ');
			echo (adjust);
			echo (', "Density resolution", ');
			echo (n);
			echo (', "Density Remove missing values", ');
			echo (narm);
			echo (' ');
		}
		echo (' ' + getValue ("histogram_opt.code.preprocess") + '))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	echo ('	hist (' + x + getValue ("histogram_opt.code.calculate") + getValue ("histogram_opt.code.printout") + getValue ("plotoptions.code.printout") + ')\n');
	if ((densityscaled) && getValue ("density")) {
		echo ('	lines(density(' + x + ', bw="' + bw + '", adjust = ' + adjust + ', ' + narm + ', n = ' + getValue ("n") + ')' + getValue ("col_density.code.printout") + ')\n');
	}
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


