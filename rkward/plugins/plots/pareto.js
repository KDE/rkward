function preprocess () {
	echo ('require(qcc)\n');
}

function calculate () {
	if (!getValue ("tabulate.checked")) {
		echo ('x <- ' + getValue ("x") + '\n');
		echo ('title <- rk.get.description (' + getValue ("x") + ')\n');
		echo ('if (!is.numeric (x)) {\n');
		echo ('	warning ("Data may not be numeric, but proceeding as requested.\\nDid you forget to check the tabulate option?")\n');
		echo ('}\n');
	} else {
		echo (getValue ('tabulate_options.code.calculate'));
	}

	if (getValue ("limit.checked")) {
		echo ('max.categories <- ' + getValue ("cutoff") + '\n');
		echo ('if (length (x) > max.categories) {\n');
		echo ('\tx <- sort (x');
		if (getValue ("sorting") != "lowest") echo (', decreasing=TRUE');
		echo (')\n');
		echo ('\tx <- c (x[1:max.categories], ' + quote (getValue ("others_label")) + '=sum (x[(max.categories+1):length(x)]))\n');
		echo ('}\n');
	}
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	var descriptives = getValue ("descriptives")=="TRUE";

	if (full) {
		echo ('rk.header ("Pareto chart")\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	echo ('\t');
	if (full && descriptives) echo ('descriptives <- ');
	echo ('pareto.chart(x' + getValue ("plotoptions.code.printout") + ')\n');
	if (full && descriptives) echo ('	rk.results(data.frame(descriptives))\n');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

