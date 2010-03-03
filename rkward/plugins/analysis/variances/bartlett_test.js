// globals
var vars;

function calculate () {
	vars = trim (getValue ("x")).replace (/\n/g, ", ");

	echo ('result <- bartlett.test (list (' + vars + '))\n');
}

function printout () {
	echo ('names <- rk.get.description (' + vars + ')\n');
	echo ('\n');
	echo ('rk.header (result$method)\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	\'Variables\'=names,\n');
	echo ('	\'Bartlett s K-squared\'=result$statistic,\n');
	echo ('	\'df\'=result$parameter,\n');
	echo ('	\'p-value\'=result$p.value))\n');
}

