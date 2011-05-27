function preprocess () {
	echo ('require(nortest)\n');
}

function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	if (getValue ("length")) {
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('	try ({\n');
	echo ('		test <- cvm.test (var)\n');
	echo ('		results[i, \'Statistic\'] <- paste (names (test$statistic), test$statistic, sep=" = ")\n');
	echo ('		results[i, \'p-value\'] <- test$p.value\n');
	echo ('	})\n');
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Cramer-von Mises Normality Test")\n');
	echo ('rk.results (results)\n');
}
