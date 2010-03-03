// globals
var vars;

function calculate () {
	vars = trim (getValue ("x")).replace (/\n/g, ", ");

	echo ('result <- fligner.test (list (' + vars + '))\n');
}

function printout () {
	echo ('names <- rk.get.description (' + vars + ')\n');
	echo ('\n');
	echo ('rk.header (result$method)\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	\'Variables\'=names,\n');
	echo ('	\'Fligner-Killeen:med X^2 test statistic\'=result$statistic,\n');
	echo ('	\'df\'=result$parameter,\n');
	echo ('	\'p-value\'=result$p.value))\n');
}


