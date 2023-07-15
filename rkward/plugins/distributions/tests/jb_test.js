/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_test_common.js");

function testCall () {
	if (getBoolean ("excludenas.state")) {
		return ("jarque.bera.test (na.omit (var))");
	}
	return ("jarque.bera.test (var)");
}

dfCall = function () {
	return ('results[i, \'df\'] <- test$parameter\n');
}

preprocess = function () {
	echo ('require (tseries)\n');	// instead of nortest
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Jarque-Bera Normality Test")).addFromUI ("excludenas").print ();
	}
	echo ('rk.results (results)\n');
}
