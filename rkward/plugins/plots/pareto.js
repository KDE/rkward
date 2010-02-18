function preprocess () {
	echo ('require(qcc)\n');
	if (getValue ("descriptives")=="TRUE") {
		echo ('require(xtable)\n');
	}
}


function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getValue ("x");
	var descriptives = getValue ("descriptives")=="TRUE";
	var tabulate = getValue ("tabulate")=="TRUE";

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
		echo ('rk.header ("Pareto chart")\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	echo ('	descriptives <- pareto.chart(x' + getValue ("plotoptions.code.printout") + ')\n');
	if (full && descriptives) {
		echo ('	rk.results(xtable(descriptives))\n');
	}
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

