function calculate () {
	var vars = trim (getValue ("x")).replace (/\n/g, " + ");
	var intercept = "";
	if (!getValue ("intercept.state.numeric")) intercept = "0 + ";

	echo ('results <- summary.lm (lm (' + getValue ("y") + ' ~ ' + intercept + vars + '))\n');
}

function printout () {
	echo ('rk.header ("Linear Regression")\n');
	echo ('rk.print(results)\n');
}

