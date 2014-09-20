function calculate () {
	var vars = trim (getValue ("x")).replace (/\n/g, " + ");
	var intercept = "";
	if (!getBoolean ("intercept")) intercept = "0 + ";

	var savefitted = getBoolean ("savefitted.active");
	var saveresiduals = getBoolean ("saveresiduals.active");
	var simple_mode = !(savefitted || saveresiduals);

	model = 'lm (' + getValue ("y") + ' ~ ' + intercept + vars;
	if (!simple_mode) model += ', na.action=na.exclude';	// default action of na.omit is a nuisance for fitted values
	model += ')';

	if (simple_mode) {
		echo ('results <- summary.lm (' + model + ')\n');
	} else {
		echo ('model <- ' + model + '\n');
		if (savefitted) echo ('.GlobalEnv$' + getString ('savefitted') + ' <- fitted (model)\n');
		if (saveresiduals) echo ('.GlobalEnv$' + getString ('saveresiduals') + ' <- residuals (model)\n');
		echo ('results <- summary.lm (model)\n');
	}
}

function printout () {
	echo ('rk.header ("Linear Regression")\n');
	echo ('rk.print(results)\n');
}

