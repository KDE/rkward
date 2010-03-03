function preprocess () {
	echo ('require(moments)\n');
}

function calculate () {
	var vars = "substitute (" + trim (getValue ("x")).replace (/\n/g, "), substitute (") + ")";
	var narm = "";
	if (getValue ("narm")) narm = ", na.rm=TRUE";
	else narm = ", na.rm=FALSE";

        echo ('objects <- list (' + vars + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=rep (NA, length (objects)), check.names=FALSE)\n');
	echo ('for (i in 1:length (objects)) {\n');
	echo ('	var <- eval (objects[[i]], envir=globalenv ())\n');
	echo ('	results[i, \'Variable Name\'] <- rk.get.description (objects[[i]], is.substitute=TRUE)\n');
	echo ('\n');
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
