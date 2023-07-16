/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	echo ('result <- mood.test (' + getValue ("x") + ', ' + getValue ("y") + ', alternative = "' + getValue ("alternative") + '")\n');
}

function printout () {
	echo ('names <- rk.get.description (' + getValue ("x") + ', ' + getValue ("y") + ')\n');
	echo ('\n');
	echo ('rk.header (result$method,\n');
	echo ('	parameters=list (' + i18n ("Alternative Hypothesis") + '=rk.describe.alternative (result)))\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variables") + '=names,\n');
	echo ('	\'Z\'=result$statistic,\n');
	echo ('	' + i18n ("p-value") + '=result$p.value))\n');
}

