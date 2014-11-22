function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (' + i18n ("Variable Name") + '=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	if (getValue ("length")) {
		echo ('	results[i, ' + i18n ("Length") + '] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo ('\n');
	}
	if (getValue ("narm")) {
		echo ('	var <- var[!is.na (var)] 	# remove NAs\n');
	}
	echo ('	try ({\n');
	echo ('		test <- PP.test (var, lshort = ' + getValue ("lshort") + ')\n');
	echo ('		results[i, ' + i18n ("Dickey-Fuller") + '] <- test$statistic\n');
	echo ('		results[i, ' + i18n ("Truncation lag parameter") + '] <- test$parameter\n');
	echo ('		results[i, ' + i18n ("p-value") + '] <- test$p.value\n');
	echo ('	})\n');
	echo ('}\n');
}

function printout () {
	new Header ("Phillips-Perron Test for Unit Roots").addFromUI ("lshort").print ();
	echo ('\n');
	echo ('rk.results (results)\n');
}

