/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- dixon.test (var, type = ' + getValue ("type") + ', opposite = ' + getValue ("opposite") + ', two.sided = ' + getValue ("two_sided") + ')\n');
	echo ('		results[i, ' + i18n ("Dixon Q-statistic") + '] <- t$statistic["Q"]\n');
	echo ('		results[i, ' + i18n ("p-value") + '] <- t$p.value\n');
	echo ('		results[i, ' + i18n ("Alternative Hypothesis") + ']<- rk.describe.alternative (t)\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Dixon test for outlier")).addFromUI ("type").addFromUI ("opposite").addFromUI ("two_sided").print ();
	}
	echo ('rk.results (results)\n');
}


