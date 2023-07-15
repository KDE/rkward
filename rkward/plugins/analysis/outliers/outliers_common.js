/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(outliers)\n');
}

function calculate () {
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (' + i18n ("Variable Name") + '=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	if (getValue ("length")) {
		echo ('	var <- vars[[i]]\n');
		echo ('\n');
		echo ('	results[i, \'Length\'] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo ('\n');
		echo ('	var <- na.omit (var) 	'); comment ("omit NAs for all further calculations");
	} else {
		echo ('	var <- na.omit (vars[[i]])\n');
	}
	echo ('\n');
	var error_column = i18n ("Error");
	echo ('	results[i, ' + error_column + '] <- tryCatch ({\n');
	comment ("This is the core of the calculation", "		");

	makeTestCall ();

	if (getValue ("descriptives")) {
		echo ('		results[i, ' + i18n ("Mean") + '] <- mean (var)\n');
		echo ('		results[i, ' + i18n ("Standard Deviation") + '] <- sd (var)\n');
		echo ('		results[i, ' + i18n ("Median") + '] <- median (var)\n');
		echo ('		results[i, ' + i18n ("Minimum") + '] <- min (var)\n');
		echo ('		results[i, ' + i18n ("Maximum") + '] <- max (var)\n');
	}
	echo ('		NA				'); comment ("no error");
	echo ('	}, error=function (e) e$message)	'); comment ("catch any errors");
	echo ('}\n');
	echo ('if (all (is.na (results$' + error_column + '))) results$' + error_column + ' <- NULL\n');
}

