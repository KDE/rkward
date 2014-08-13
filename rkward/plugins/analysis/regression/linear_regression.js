function calculate () {
	var vars = trim (getValue ("x")).replace (/\n/g, " + ");
	var intercept = "";
	if (!getBoolean ("intercept")) intercept = "0 + ";

	var savefitted = getBoolean ("savefitted.active");
	var simple_mode = !savefitted;

	model = 'lm (' + getValue ("y") + ' ~ ' + intercept + vars;
	if (savefitted) model += ', na.action=na.exclude';	// default action of na.omit is a nuisance for fitted values
	model += ')';

	if (simple_mode) {
		echo ('results <- summary.lm (' + model + ')\n');
	} else {
		echo ('model <- ' + model + '\n');
		echo ('.GlobalEnv$' + getString ('savefitted') + ' <- fitted (model)\n');
		echo ('results <- summary.lm (model)\n');
	}
}

function printout () {
	echo ('rk.header ("Linear Regression")\n');
	echo ('rk.print(results)\n');
}

