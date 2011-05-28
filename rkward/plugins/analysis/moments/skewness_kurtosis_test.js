function preprocess () {
	echo ('require(moments)\n');
}

function calculate () {
	var narm = ", na.rm=FALSE";
	if (getValue ("narm")) narm = ", na.rm=TRUE";
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	echo ('	try ({\n');
	if (getValue ("skewness")) {
		echo ('		results[i, \'Skewness\'] <- skewness (var' + narm + ')\n');
	}
	if (getValue ("kurtosis")) {
		echo ('		results[i, \'Kurtosis\'] <- kurtosis (var' + narm + ')\n');
		echo ('		results[i, \'Excess Kurtosis\'] <- results[i, \'Kurtosis\'] - 3\n');
	}
	if (getValue ("geary")) {
		echo ('		results[i, \'Geary Kurtosis\'] <- geary (var' + narm + ')\n');
	}
	echo ('	})\n');
	if (getValue ("length")) {
		echo ('\n');
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Skewness and Kurtosis")\n');
	echo ('rk.results (results)\n');
}
