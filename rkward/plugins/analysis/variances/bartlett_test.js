// globals
var vars;

function calculate () {
	vars = getList ("x").join (", ");

	echo ('result <- bartlett.test (list (' + vars + '))\n');
}

function printout () {
	echo ('names <- rk.get.description (' + vars + ')\n');
	echo ('\n');
	echo ('rk.header (result$method)\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variables") + '=names,\n');
	echo ('	' + i18n ("Bartlett's K-squared") + '=result$statistic,\n');
	echo ('	\'df\'=result$parameter,\n');
	echo ('	' + i18n ("p-value") + '=result$p.value))\n');
}

