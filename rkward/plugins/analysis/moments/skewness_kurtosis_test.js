/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("moments_common.js");
function preprocess () {} // empty for this one

function insertTestCall () {
	var narm = ", na.rm=FALSE";
	if (getValue ("narm")) narm = ", na.rm=TRUE";

	if (getValue ("skewness")) {
		echo ('		results[i, ' + i18n ("Skewness") +'] <- skewness (var' + narm + ')\n');
	}
	if (getValue ("kurtosis")) {
		echo ('		results[i, ' + i18n ("Kurtosis") + '] <- kurtosis (var' + narm + ')\n');
		echo ('		results[i, ' + i18n ("Excess Kurtosis") + '] <- results[i, \'Kurtosis\'] - 3\n');
	}
	if (getValue ("geary")) {
		echo ('		results[i, ' + i18n ("Geary Kurtosis") + '] <- geary (var' + narm + ')\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Skewness and Kurtosis") + ')\n');
	}
	echo ('rk.results (results)\n');
}
