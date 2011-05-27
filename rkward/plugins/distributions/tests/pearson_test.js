function preprocess () {
	echo ('require(nortest)\n');
}

function calculate () {
	var adjust = getValue ("adjust");
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
	echo ('		test <- pearson.test (var, ' +  adjust + ')\n');
	echo ('		results[i, \'Statistic\'] <- paste (names (test$statistic), test$statistic, sep=" = ")\n');
	echo ('		results[i, \'p-value\'] <- test$p.value\n');
	echo ('		results[i, \'number of classes\'] <- test$n.classes\n');
	echo ('		results[i, \'degrees of freedom\'] <- test$df\n');
	echo ('	})\n');
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Pearson chi-square Normality Test",\n');
	echo ('	parameters=list ("chi-square distribution with n.classes-3 df (TRUE) or chi-square distribution with n.classes-1 df (FALSE)", "' + getValue ("adjust") + '"))\n');
	echo ('rk.results (results)\n');
}
