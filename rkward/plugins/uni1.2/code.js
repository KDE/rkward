function calculate () {
	var narm = "na.rm=FALSE";
	if (getValue ("narm")) narm = "na.rm=TRUE";

	var vars = trim (getValue ("z"));
	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	echo ('\n');
	if (getValue ("length")) {
		echo ('	results[i, \'Number of obs\'] <- length(var)\n');
		echo ('	results[i, \'Number of missing values\'] <- sum(is.na(var))\n');
	}
	if (getValue ("mean")) {
		echo ('	results[i, \'Mean\'] <- mean(var,' + narm + ')\n');
	}
	if (getValue ("vari")) {
		echo ('	results[i, \'Variance\'] <- var(var,' + narm + ')\n');
	}
	if (getValue ("sd")) {
		echo ('	results[i, \'Sd\'] <- sd(var,' + narm + ')\n');
	}
	if (getValue ("minimum")) {
		echo ('	results[i, \'Minimum\'] <- min(var,' + narm + ')\n');
	}
	if (getValue ("maximum")) {
		echo ('	results[i, \'Maximum\'] <- max(var,' + narm + ')\n');
	}
	var nmin;
	if ((nmin = getValue ("nbminimum")) != "0") {
		echo ('	if (length (var) >= ' + nmin + ') {\n');
		echo ('		results[i, \'Minimum values\'] <- paste (sort(var, decreasing=FALSE, na.last=TRUE)[1:' + nmin + '], collapse=" ")\n');
		echo ('	}\n');
	}
	var nmax;
	if ((nmax = getValue ("nbmaximum")) != "0") {
		echo ('	if (length (var) >= ' + nmax + ') {\n');
		echo ('		results[i, \'Maximum values\'] <- paste (sort(var, decreasing=TRUE, na.last=TRUE)[1:' + nmax + '], collapse=" ")\n');
		echo ('	}\n');
	}
	if (getValue ("median")) {
		echo ('	results[i, \'Median\'] <- median(var,' + narm + ')\n');
	}
	if (getValue ("irq")) {
		echo ('	results[i, \'Inter Quartile Range\'] <- IQR(var,' + narm + ')\n');
	}
	if (getValue ("quartile")) {
		echo ('	temp <- quantile (var,' + narm + ')\n');
		echo ('	results[i, \'Quartiles\'] <- paste (names (temp), temp, sep=": ", collapse=" ")\n');
	}
	var nautre;
	if ((nautre = getValue ("autre")) != "0") {
		echo ('	temp <- quantile (var, probs=seq (0, 1, length.out=' + nautre + '), ' + narm + ')\n');
		echo ('	results[i, \'Quantiles\'] <- paste (names (temp), temp, sep=": ", collapse=" ")\n');
	}
	echo ('	\n');
	echo ('	#robust statistics\n');
	if (getValue ("trim") == "1") {
		echo ('	results[i, \'Trimmed Mean\'] <- mean (var, trim=' + getValue ("pourcent") + ', ' + narm + ')\n');
	}
	if (getValue ("mad") == "1") {
		echo ('	results[i, \'Median Absolute Deviation\'] <- mad (var, constant=' + getValue ("constMad") + ', ' + narm + ')\n');
	}
	if (getValue ("huber") == "1") {
		echo ('	require ("MASS")\n');
		echo ('	temp <- list (c(\'Location Estimate\',\'Mad scale estimate\'), c(NA,NA))\n');
		echo ('	try({\n');
		echo ('		temp <- hubers (var, k = ' + getValue ("winsor") + ',tol=' + getValue ("tol"));
		if (getValue("customMu")=="1") echo (", mu="+getValue("mu"));
		if (getValue("customS")=="1") echo (", s="+getValue("s"));
		echo (",initmu ="+getValue("initmu")+"(var))\n");
		echo ('	})\n');
		echo ('	results[i, \'Huber M-Estimator\'] <- paste (temp[[1]], temp[[2]], sep=": ", collapse=" ")\n');
	}
	echo ('}\n');
	echo ('\n');
	if (getValue ("saveas.active")) {
		echo ('# store results\n');
		echo ('.GlobalEnv$' + getValue ("saveas") + ' <- results\n');
	}
}

function printout () {
	echo ('rk.header ("Univariate statistics", parameters=list (\n');
	echo ('"Remove Missing values", ');
	if (getValue ("narm")) echo ("TRUE");
	else echo ("FALSE");
	if (getValue("trim")=="1") {
		echo (', "Trimmed value for trimmed mean", "' + getValue ("pourcent") + '"\n');
	}
	if (getValue("mad")=="1") {
		echo (', "Constant for the MAD estimation", "' + getValue ("constMad") + '"\n');
	}
	if (getValue("huber")=="1") {
		echo (', "Winsorized values for Huber estimator", "' + getValue ("winsor") + '"\n');
		echo (', "Tolerance in Huber estimator", "' + getValue ("tol") + '"\n');
		if (getValue ("customMu")=="1") {
			echo (', "Mu for Huber estimator", "' + getValue ("mu") + '"\n');
		}
		if (getValue ("customS")=="1") {
			echo (', "S for Huber estimator", "' + getValue ("s") + '"\n');
		}
		echo (', "Initial value", "' + getValue ("initmu") + '"\n');
	}
	echo ('))\n');
	echo ('\n');
	echo ('rk.results (results)\n');
}

