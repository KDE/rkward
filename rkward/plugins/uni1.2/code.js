/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	var narm = "na.rm=FALSE";
	if (getValue ("narm")) narm = "na.rm=TRUE";

	var vars = getList ("z");
	echo ('vars <- rk.list (' + vars.join (", ") + ')\n');
	echo ('results <- data.frame (' + i18n ("Variable Name") + '=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	echo ('\n');
	if (getValue ("length")) {
		echo ('	results[i, ' + i18n ("Number of cases") + '] <- length(var)\n');
		echo ('	results[i, ' + i18n ("Number of missing values") + '] <- sum(is.na(var))\n');
	}
	if (getValue ("mean")) {
		echo ('	results[i, ' + i18n ("Mean") + '] <- mean(var,' + narm + ')\n');
	}
		if (getValue ("geo_mean")) {
		// compute the geometric mean
		echo ('	results[i, ' + i18n ("geometric mean") + '] <- try (prod (na.omit(var))^(1 / length (na.omit(var))))\n');
	}
	if (getValue ("interquantile_mean")) {
		// compute the quartile (25% and 75%) mean
		echo ('	results[i, ' + i18n ("interquantile mean") + '] <- try (sum(quantile(var, probs=c(0.25), na.rm=T), quantile(var, probs=c(0.75), na.rm=TRUE)) / 2)\n');
	}
	if (getValue ("harmonic_mean")) {
		// compute the harmonic mean
		echo ('	results[i, ' + i18n ("harmonic mean") + '] <- try (1 / mean(1 / na.omit(var)))\n');
	}
	if (getValue ("vari")) {
		echo ('	results[i, ' + i18n ("Variance") + '] <- var(var,' + narm + ')\n');
	}
	if (getValue ("sd")) {
		echo ('	results[i, ' + i18nc ("standard deviation; short", "sd") + '] <- sd(var,' + narm + ')\n');
	}
	if (getValue ("minimum")) {
		echo ('	results[i, ' + i18n ("Minimum") + '] <- min(var,' + narm + ')\n');
	}
	if (getValue ("maximum")) {
		echo ('	results[i, ' + i18n ("Maximum") + '] <- max(var,' + narm + ')\n');
	}
	var nmin;
	if ((nmin = getValue ("nbminimum")) != "0") {
		echo ('	if (length (var) >= ' + nmin + ') {\n');
		echo ('		results[i, ' + i18n ("Minimum values") + '] <- paste (format(sort(var, decreasing=FALSE, na.last=TRUE)[1:' + nmin + ']), collapse=" ")\n');
		echo ('	}\n');
	}
	var nmax;
	if ((nmax = getValue ("nbmaximum")) != "0") {
		echo ('	if (length (var) >= ' + nmax + ') {\n');
		echo ('		results[i, ' + i18n ("Maximum values") + '] <- paste (format(sort(var, decreasing=TRUE, na.last=TRUE)[1:' + nmax + ']), collapse=" ")\n');
		echo ('	}\n');
	}
	if (getValue ("median")) {
		echo ('	results[i, ' + i18n ("Median") + '] <- median(var,' + narm + ')\n');
	}
	if (getValue ("irq")) {
		echo ('	results[i, ' + i18n ("Inter Quartile Range") + '] <- IQR(var,' + narm + ')\n');
	}
	if (getValue ("quartile")) {
		echo ('	temp <- quantile (var,' + narm + ')\n');
		echo ('	results[i, ' + i18n ("Quartiles") + '] <- paste (names (temp), format (temp), sep=": ", collapse=" ")\n');
	}
	var nautre;
	if ((nautre = getValue ("autre")) != "0") {
		echo ('	temp <- quantile (var, probs=seq (0, 1, length.out=' + nautre + '), ' + narm + ')\n');
		echo ('	results[i, ' + i18n ("Quantiles") + '] <- paste (names (temp), format (temp), sep=": ", collapse=" ")\n');
	}
	echo ('	\n');
	comment ("robust statistics", "	");
	if (getValue ("trim") == "1") {
		echo ('	results[i, ' + i18n ("Trimmed Mean") + '] <- mean (var, trim=' + getValue ("pourcent") + ', ' + narm + ')\n');
	}
	if (getValue ("mad") == "1") {
		echo ('	results[i, ' + i18n ("Median Absolute Deviation") + '] <- mad (var, constant=' + getValue ("constMad") + ', ' + narm + ')\n');
	}
	if (getValue ("huber") == "1") {
		echo ('	require ("MASS")\n');
		echo ('	temp <- list (c(' + i18n ("Location Estimate") + ',' + i18n ("Mad scale estimate") + '), c(NA,NA))\n');
		echo ('	try({\n');
		echo ('		temp <- hubers (var, k = ' + getValue ("winsor") + ',tol=' + getValue ("tol"));
		if (getValue("customMu")=="1") echo (", mu="+getValue("mu"));
		if (getValue("customS")=="1") echo (", s="+getValue("s"));
		echo (",initmu ="+getValue("initmu")+"(var))\n");
		echo ('	})\n');
		echo ('	results[i, ' + i18n ("Huber M-Estimator") + '] <- paste (format (temp[[1]]), format (temp[[2]]), sep=": ", collapse=" ")\n');
	}
	echo ('}\n');
	echo ('\n');
	if (getValue ("saveas.active")) {
		comment ('store results');
		echo ('.GlobalEnv$' + getValue ("saveas") + ' <- results\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		header = new Header (i18n ("Univariate statistics")).addFromUI ("narm");
		if (getBoolean("trim.state")) {
			header.add (i18n ("Proportion of trimmed values for trimmed mean"), getString ("pourcent"));
		}
		if (getBoolean("mad.state")) {
			header.add (i18n ("Constant for the MAD estimation"), getString ("constMad"));
		}
		if (getBoolean("huber.state")) {
			header.add (i18n ("Winsorized values for Huber estimator"), getString ("winsor"));
			header.add (i18n ("Tolerance in Huber estimator"), getString ("tol"));
			if (getBoolean ("customMu.state")) {
				header.add (i18n ("Mu for Huber estimator"), getString ("mu"));
			}
			if (getBoolean ("customS.state")) {
				header.add (i18n ("S for Huber estimator"), getString ("s"));
			}
			header.add (i18n ("Initial value"), getString ("initmu"));
		}
		header.print ();
		echo ('\n');
	}
	echo ('rk.results (results)\n');
	if (getValue ("save_to_file")) echo ('write.csv(file="' + getValue ("file") + '", results)\n');
}

