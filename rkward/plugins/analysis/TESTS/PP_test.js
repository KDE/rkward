function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	if (getValue ("length")) {
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo ('\n');
	}
	if (getValue ("narm")) {
		echo ('	var <- var[!is.na (var)] 	# remove NAs\n');
	}
	echo ('	try ({\n');
	echo ('		test <- PP.test (var, lshort = ' + getValue ("lshort") + ')\n');
	echo ('		results[i, \'Dickey-Fuller\'] <- test$statistic\n');
	echo ('		results[i, \'Truncation lag parameter\'] <- test$parameter\n');
	echo ('		results[i, \'p-value\'] <- test$p.value\n');
	echo ('	})\n');
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Phillips-Perron Test for Unit Roots",\n');
	echo ('	parameters=list ("Truncation lag parameter short (\'TRUE\') or long (\'FALSE\')", "' + getValue ("lshort") + '"))\n');
	echo ('\n');
	echo ('rk.results (results)\n');
}

