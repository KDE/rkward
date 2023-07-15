/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("moments_common.js");

function insertTestCall () {
	echo ('		t <- anscombe.test (var, alternative = "' + getValue ("alternative") + '")\n');
	echo ('		results[i, \'Kurtosis estimator (tau)\'] <- t$statistic["kurt"]\n');
	echo ('		results[i, \'Transformation (z)\'] <- t$statistic["z"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	if (getValue ("show_alternative")) {
		echo ('		results[i, \'Alternative Hypothesis\'] <- rk.describe.alternative (t)\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Anscombe-Glynn test of kurtosis")).addFromUI ("alternative").print ();
	}
	echo ('rk.results (results)\n');
}

