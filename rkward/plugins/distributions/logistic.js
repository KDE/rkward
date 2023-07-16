/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_common.js");

function getDistSpecifics () {
	var loc = Number (getString ("location"));
	var scale = Number (getString ("scale"));
	return initDistSpecifics (i18n ('Logistic distribution'), 'logis', ["location", "scale"], [-5*scale+loc, 5*scale+loc], continuous);
}
