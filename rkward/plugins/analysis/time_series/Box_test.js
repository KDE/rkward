/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate() {
	var vars = trim(getValue("x"));

	echo('vars <- rk.list (' + vars.split("\n").join(", ") + ')\n');
	echo('results <- data.frame (' + i18n("Variable Name") + '=I(names (vars)), check.names=FALSE)\n');
	echo('for (i in 1:length (vars)) {\n');
	echo('	var <- vars[[i]]\n');
	if (getValue("length")) {
		echo('	results[i, ' + i18n("Length") + '] <- length (var)\n');
		echo('	results[i, \'NAs\'] <- sum (is.na(var))\n');
		echo('\n');
	}
	if (getValue("narm")) {
		echo('	var <- var[!is.na (var)] 	# remove NAs\n');
	}
	echo('	try ({\n');
	echo('		test <- Box.test (var, lag = ' + getValue("lag") + ', type = "' + getValue("type") + '")\n');
	echo('		results[i, ' + i18n("X-squared") + '] <- test$statistic\n');
	echo('		results[i, ' + i18n("degrees of freedom") + '] <- test$parameter\n');
	echo('		results[i, ' + i18n("p-value") + '] <- test$p.value\n');
	echo('	})\n');
	echo('}\n');
}

function printout(is_preview) {
	if (!is_preview) {
		new Header(i18n("Box-Pierce and Ljung-Box Tests"))
		    .addFromUI("lag")
		    .addFromUI("type")
		    .print();
	}
	echo('\n');
	echo('rk.results (results)\n');
}
