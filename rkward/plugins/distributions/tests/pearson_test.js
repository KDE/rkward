/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_test_common.js");

function testCall () {
	return ("pearson.test (var, " + getString ("adjust") + ")");
}

dfCall = function () {
	return ('results[i, ' + i18n ("number of classes") + '] <- test$n.classes\n' +
	        'results[i, ' + i18n ("degrees of freedom") + '] <- test$df\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Pearson chi-square Normality Test")).addFromUI ("adjust").print ();
	}
	echo ('rk.results (results)\n');
}
