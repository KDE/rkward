/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var vars;

function calculate () {
	vars = getList ("x").join (", ");

	echo ('result <- fligner.test (list (' + vars + '))\n');
}

function printout (is_preview) {
	echo ('names <- rk.get.description (' + vars + ')\n');
	echo ('\n');
	if (!is_preview) {
		echo ('rk.header (result$method)\n');
		echo ('\n');
	}
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variables") + '=names,\n');
	echo ('	' + i18n ("Fligner-Killeen:med X^2 test statistic") + '=result$statistic,\n');
	echo ('	\'df\'=result$parameter,\n');
	echo ('	' + i18n ("p-value") + '=result$p.value))\n');
}


