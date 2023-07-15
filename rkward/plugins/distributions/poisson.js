/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_common.js");

function getDistSpecifics () {
	var lambda = Number (getString ("lambda"));
	return initDistSpecifics (i18n ('Poisson distribution'), 'pois', ["lambda"], [0, lambda + Math.sqrt (lambda)*3 + 1], discrete);
}
