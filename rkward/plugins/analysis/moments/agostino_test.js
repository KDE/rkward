/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("moments_common.js");

function insertTestCall () {
	echo ('		# This is the core of the calculation\n');
	echo ('		t <- agostino.test (var, alternative = "' + getValue ("alternative") + '")\n');
	echo ('		results[i, \'skewness estimator (skew)\'] <- t$statistic["skew"]\n');
	echo ('		results[i, \'transformation (z)\'] <- t$statistic["z"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	if (getValue ("show_alternative")) {
		echo ('		results[i, \'Alternative Hypothesis\'] <- rk.describe.alternative (t)\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("D'Agostino test of skewness")).addFromUI ("alternative").print ();
	}
	echo ('rk.results (results)\n');
}


