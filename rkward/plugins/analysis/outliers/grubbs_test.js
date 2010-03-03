function preprocess () {
	echo ('require(outliers)\n');
}

function calculate () {
	var vars = "substitute (" + trim (getValue ("x")).replace (/\n/g, "), substitute (") + ")";

	echo ('\n');
	echo ('vars <- list (' + vars + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=rep (NA, length (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length(vars)) {\n');
	echo ('	results[i, \'Variable Name\'] <- rk.get.description (vars[[i]], is.substitute=TRUE)\n');
	if (getValue ("length")) {
		echo ('	var <- eval (vars[[i]], envir=globalenv ())\n');
		echo ('\n');
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo ('\n');
		echo ('	var <- na.omit (var) 	# omit NAs for all further calculations\n');
	} else {
		echo ('	var <- na.omit (eval (vars[[i]], envir=globalenv ()))\n');
	}
	echo ('\n');
	echo ('	results[i, \'Error\'] <- tryCatch ({\n');
	echo ('		# This is the core of the calculation\n');
	echo ('		t <- grubbs.test (var, type = ' + getValue ("type") + ', opposite = ' + getValue ("opposite") + ', two.sided = ' + getValue ("two_sided") + ')\n');
	echo ('		results[i, \'G\'] <- t$statistic["G"]\n');
	echo ('		results[i, \'U\'] <- t$statistic["U"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	echo ('		results[i, \'Alternative Hypothesis\']<- rk.describe.alternative (t)\n');
	if (getValue ("descriptives")) {
		echo ('		results[i, \'Mean\'] <- mean (var)\n');
		echo ('		results[i, \'Standard Deviation\'] <- sd (var)\n');
		echo ('		results[i, \'Median\'] <- median (var)\n');
		echo ('		results[i, \'Minimum\'] <- min (var)\n');
		echo ('		results[i, \'Maximum\'] <- max (var)\n');
	}
	echo ('		NA				# no error\n');
	echo ('	}, error=function (e) e$message)	# catch any errors\n');
	echo ('}\n');
	echo ('if (all (is.na (results$\'Error\'))) results$\'Error\' <- NULL\n');
}

function printout () {
	echo ('rk.header ("Grubbs tests for one or two outliers in data sample",\n');
	echo ('	parameters=list ("Type", "' + getValue ("type") + '", "Opposite", "' + getValue ("opposite") + '", "two-sided", "' + getValue ("two_sided") + '"))\n');
	echo ('rk.results (results)\n');
}


