function preprocess () {
	echo ('require(moments)\n');
}

function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	echo ('	try (results[i, \'Moment\'] <- moment (var, order = ' + getValue ("order") + ', central = ' + getValue ("central") + ', absolute = ' + getValue ("absolute") + ', na.rm = ' + getValue ("narm") + '))\n');
	if (getValue ("length")) {
		echo ('\n');
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Statistical Moment",\n');
	echo ('	parameters=list ("Order", "' + getValue ("order") + '", "Compute central moments", "' + getValue ("central") + '", "Compute absolute moments", "' + getValue ("absolute") + '", "Remove missing values", "' + getValue ("narm") + '"))\n');
	echo ('rk.results (results)\n');
}

