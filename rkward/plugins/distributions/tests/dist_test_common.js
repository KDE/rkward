function preprocess () {
	echo ('require (nortest)\n');
}

function dfCall () {
	return ('');
}

function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (' + i18n ("Variable Name") + '=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	if (getValue ("length")) {
		echo ('	results[i, ' + i18n ("Length") + '] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('	try ({\n');
	echo ('		test <- ' + testCall () + '\n');
	echo ('		results[i, ' + i18nc ("Statistic indicator", "Statistic") + '] <- paste (names (test$statistic), format (test$statistic), sep=" = ")\n');
	printIndentedUnlessEmpty ('		', dfCall ());
	echo ('		results[i, ' + i18n ("p-value") + '] <- test$p.value\n');
	echo ('	})\n');
	echo ('}\n');
}
 