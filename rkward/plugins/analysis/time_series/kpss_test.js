function preprocess () {
	echo ('require (tseries)\n');
}

function calculate () {
	var vars = "substitute (" + trim (getValue ("x")).replace (/\n/g, "), substitute (") + ")";

	echo ('objects <- list (' + vars + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=rep (NA, length (objects)), check.names=FALSE)\n');
	echo ('for (i in 1:length (objects)) {\n');
	echo ('	results[i, \'Variable Name\'] <- rk.get.description (objects[[i]], is.substitute=TRUE)\n');
	echo ('	var <- eval (objects[[i]], envir=globalenv ())\n');
	if (getValue ("length")) {
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo ('\n');
	}
	if (getValue ("narm")) {
		echo ('	var <- var[!is.na (var)] 	# remove NAs\n');
	}
	echo ('	try ({\n');
	echo ('		test <- kpss.test (var, null = "' + getValue ("null") + '", lshort = ' + getValue ("lshort") + ')\n');
	echo ('		results[i, \'KPSS ' + getValue ("null") + '\'] <- test$statistic\n');
	echo ('		results[i, \'Truncation lag parameter\'] <- test$parameter\n');
	echo ('		results[i, \'p-value\'] <- test$p.value\n');
	echo ('	})\n');
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("KPSS Test for Level Stationarity",\n');
	echo ('	parameters=list ("null hypothesis"="' + getValue ("null") + '", "version of truncation lag parameter"="');
	if (getValue ("lshort") == "TRUE") echo ("short");
	else echo ("long" + '"))\n');
	echo ('\n');
	echo ('rk.results (results)\n');
}

