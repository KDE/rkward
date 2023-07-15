/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	echo ('result <- var.test (' + getValue ("x") + ', ' + getValue ("y") + ', alternative = "' + getValue ("alternative") + '", ratio = ' + getValue ("ratio"));
	var conflevel = getValue ("conflevel");
	if (conflevel != "0.95") echo (", conf.level=" + conflevel);
	echo (')\n');
	echo ('\n');
}

function printout (is_preview) {
	echo ('names <- rk.get.description (' + getValue ("x") + ', ' + getValue ("y") + ')\n');
	echo ('\n');
	if (!is_preview) {
		new Header (noquote ('result$method'))
		    .addFromUI ("conflevel")
		    .addFromUI ("ratio")
		    .add (i18n ("Alternative Hypothesis"), noquote ('rk.describe.alternative(result)')).print ();
	}
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variables") + '=names,\n');
	echo ('	\'F\'=result$statistic["F"],\n');
	echo ('	' + i18n ("Numerator DF") + '=result$parameter["num df"],\n');
	echo ('	' + i18n ("Denominator DF") + '=result$parameter["denom df"],\n');
	echo ('	' + i18n ("p-value") + '=result$p.value,\n');
	echo ('	' + i18n ("Lower CI") + '=result$conf.int[1],\n');
	echo ('	' + i18n ("Upper CI") + '=result$conf.int[2],\n');
	echo ('	' + i18n ("ratio of variances") + '=result$estimate))\n');
}

