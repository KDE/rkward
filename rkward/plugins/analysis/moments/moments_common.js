/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(moments)\n');
}

function calculate () {
	alternative = getValue ("alternative");
	var vars = trim (getValue ("x"));

	echo ('vars <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	echo ('results <- data.frame (\'Variable Name\'=I(names (vars)), check.names=FALSE)\n');
	echo ('for (i in 1:length (vars)) {\n');
	echo ('	var <- vars[[i]]\n');
	var error_column = i18n ("Error");
	echo ('	results[i, ' + error_column + '] <- tryCatch ({\n');
	comment ("This is the core of the calculation", '		');

	insertTestCall ();

	echo ('		NA				# no error\n');
	echo ('	}, error=function (e) e$message)	'); comment ("catch any errors");
	if (getValue ("length")) {
		echo ('	results[i, ' + i18n ("Length") + '] <- length (var)\n');
		echo ('	results[i, \'NAs\'] <- sum (is.na(var))\n');
	}
	echo ('}\n');
	echo ('if (all (is.na (results$' + error_column + '))) results$' + error_column + ' <- NULL\n');
}
