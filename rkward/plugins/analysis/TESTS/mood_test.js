function calculate () {
	echo ('result <- mood.test (' + getValue ("x") + ', ' + getValue ("y") + ', alternative = "' + getValue ("alternative") + '")\n');
}

function printout () {
	echo ('names <- rk.get.description (' + getValue ("x") + ', ' + getValue ("y") + ')\n');
	echo ('\n');
	echo ('rk.header (result$method,\n');
	echo ('	parameters=list ("Alternative Hypothesis", rk.describe.alternative (result)))\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	\'Variables\'=names,\n');
	echo ('	\'Z\'=result$statistic,\n');
	echo ('	\'p-value\'=result$p.value))\n');
}

