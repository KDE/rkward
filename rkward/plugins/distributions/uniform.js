/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_common.js");

function getDistSpecifics () {
	var min = Number (getString ("min"));
	var max = Number (getString ("max"));
	return initDistSpecifics (i18n ('Uniform distribution'), 'unif', ["min", "max"], [min, max], continuous);
}
