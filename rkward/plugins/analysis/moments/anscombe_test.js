// globals
var alternative;

function preprocess () {
	echo ('require(moments)\n');
}

function calculate () {
	var vars = "substitute (" + trim (getValue ("x")).replace (/\n/g, "), substitute (") + ")";
	alternative = getValue ("alternative");

	echo ('vars <- list (' + vars + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=rep (NA, length (vars)), check.names=FALSE)\n');
	echo ('\n');
	echo ('for (i in 1:length(vars)) {\n');
	echo ('	results[i, \'Variable Name\'] <- rk.get.description (vars[[i]], is.substitute=TRUE)\n');
	echo ('	var <- eval (vars[[i]], envir=globalenv ())\n');
	echo ('	results[i, \'Error\'] <- tryCatch ({\n');
	echo ('		t <- anscombe.test (var, alternative = "' + alternative + '")\n');
	echo ('		results[i, \'Kurtosis estimator (tau)\'] <- t$statistic["kurt"]\n');
	echo ('		results[i, \'Transformation (z)\'] <- t$statistic["z"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	if (getValue ("show_alternative")) {
		echo ('		results[i, \'Alternative Hypothesis\'] <- rk.describe.alternative (t)\n');
	}
	echo ('		NA				# no error\n');
	echo ('	}, error=function (e) e$message)	# catch any errors\n');
	if (getValue ("length")) {
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('}\n');
	echo ('if (all (is.na (results$\'Error\'))) results$\'Error\' <- NULL\n');
}

function printout () {
	echo ('rk.header ("Anscombe-Glynn test of kurtosis",\n');
	echo ('	parameters=list ("Alternative Hypothesis", "' + alternative + '"))\n');
	echo ('rk.results (results)\n');
}

