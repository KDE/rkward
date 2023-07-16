/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var mad_type;
var constMad;

function calculate () {
	var vars = getList ("x");
	var groups = getList ("groups");
	constMad = getValue ("constMad");
	mad_type = getValue ("mad_type");

	echo ('vars <- rk.list (' + vars.join (", ") + ')\n');
	if (groups.length) {
		comment("Split each input variable by grouping factor(s)");
		echo('vars <- lapply (vars, function (x) split(x, list (' + groups.join(', ') + ')))\n');
		comment("Convert nested list into flat list");
		echo('vars <- unlist (vars, recursive=FALSE)\n');
	}
	echo ('results <- data.frame (' + i18n ("Object") + '=I(names (vars)))\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	echo ('\n');
	comment ('we wrap each single call in a "try" statement to always continue on errors.', '\t');
	if (getValue ("mean")) {
		// trim is the fraction (0 to 0.5) of observations to be trimmed from each end of x before the mean is computed
		echo ('	results[i, ' + i18n ("mean") + '] <- try (mean (var, trim = ' + getValue ("trim") + ', na.rm=TRUE))\n');
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
	if (getValue ("median")) {
		echo ('	results[i, ' + i18n ("median") +  '] <- try (median (var, na.rm=TRUE))\n');
	}
	if (getValue ("range")) {
		echo ('	try ({\n');
		echo ('		range <- try (range (var, na.rm=TRUE))\n');
		echo ('		results[i, ' + i18n ("min") + '] <- range[1]\n');
		echo ('		results[i, ' + i18n ("max") + '] <- range[2]\n');
		echo ('	})\n');
	}
	if (getValue ("sd")) {
		echo ('	results[i, ' + i18n ("standard deviation") + '] <- try (sd (var, na.rm=TRUE))\n');
	}
	if (getValue ("sum")) {
		echo ('	results[i, ' + i18nc ("noun", "sum") + '] <- try (sum (var, na.rm=TRUE))\n');
	}
	if (getValue ("prod")) {
		echo ('	results[i, ' + i18n ("product") + '] <- try (prod (var, na.rm=TRUE))\n');
	}
	if (getValue ("mad")) {
		echo ('	results[i, ' + i18n ("Median Absolute Deviation") + '] <- try (mad (var, constant = ' + constMad);
		if (mad_type == "low") echo (", low=TRUE");
		else if (mad_type == "high") echo (", high=TRUE");
		echo (', na.rm=TRUE))\n');
	}
	if (getValue ("length")) {
		echo ('	results[i, ' + i18n ("total length (N)") + '] <- length (var)\n');
		echo ('	results[i, ' + i18n ("number of NAs") + '] <- sum (is.na(var))\n');
	}
	echo ('}\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Descriptive statistics")).addFromUI ("trim").print ();
		if (getValue ("mad")) {
			new Header (i18n ("Median Absolute Deviation"), 3).addFromUI ("constMad").addFromUI ("mad_type").print ();
		}
	}
	echo ('rk.results (results)\n');
	if (getValue ("save_to_file")) echo ('write.csv(file="' + getValue ("file") + '", results)\n');
}
