// globals
var mad_type;
var constMad;

function calculate () {
	var vars = "substitute (" + trim (getValue ("x")).replace (/\n/g, "), substitute (") + ")";
	constMad = getValue ("constMad");
	mad_type = getValue ("mad_type");


	echo ('vars <- list (' + vars + ')\n');
	echo ('results <- data.frame (\'Object\'=rep (NA, length (vars)))\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	results[i, \'Object\'] <- rk.get.description (vars[[i]], is.substitute=TRUE)\n');
	echo ('	var <- eval (vars[[i]], envir=globalenv())	# fetch the real object\n');
	echo ('\n');
	echo ('	# we wrap each single call in a "try" statement to always continue on errors.\n');
	if (getValue ("mean")) {
		// trim is the fraction (0 to 0.5) of observations to be trimmed from each end of x before the mean is computed
		echo ('	results[i, \'mean\'] <- try (mean (var, trim = ' + getValue ("trim") + ', na.rm=TRUE))\n');
	}
	if (getValue ("median")) {
		echo ('	results[i, \'median\'] <- try (median (var, na.rm=TRUE))\n');
	}
	if (getValue ("range")) {
		echo ('	try ({\n');
		echo ('		range <- try (range (var, na.rm=TRUE))\n');
		echo ('		results[i, \'min\'] <- range[1]\n');
		echo ('		results[i, \'max\'] <- range[2]\n');
		echo ('	})\n');
	}
	if (getValue ("sd")) {
		echo ('	results[i, \'standard deviation\'] <- try (sd (var, na.rm=TRUE))\n');
	}
	if (getValue ("sum")) {
		echo ('	results[i, \'sum\'] <- try (sum (var, na.rm=TRUE))\n');
	}
	if (getValue ("prod")) {
		echo ('	results[i, \'product\'] <- try (prod (var, na.rm=TRUE))\n');
	}
	if (getValue ("mad")) {
		echo ('	results[i, \'Median Absolute Deviation\'] <- try (mad (var, constant = ' + constMad);
		if (mad_type == "low") echo (", low=TRUE");
		else if (mad_type == "high") echo (", high=TRUE");
		echo (', na.rm=TRUE))\n');
	}
	if (getValue ("length")) {
		echo ('	results[i, \'length of sample\'] <- length (var)\n');
		echo ('	results[i, \'number of NAs\'] <- sum (is.na(var))\n');
	}
	echo ('}\n');
}

function printout () {
	echo ('rk.header ("Descriptive statistics", parameters=list (\n');
	echo ('               "Trim of mean", ' + getValue ("trim"));
	if (getValue ("mad")) {
		echo (',\n');
		echo ('               "Median Absolute Deviation",\n');
		echo ('               paste ("constant:", ' + constMad + ', ');
		if (mad_type == "low") echo ('"lo-median"');
		else if (mad_type == "high") echo ('"hi-median"');
		else echo ('"average"' + ')');
	}
	echo ('))\n');
	echo ('\n');
	echo ('rk.results (results)\n');
}


