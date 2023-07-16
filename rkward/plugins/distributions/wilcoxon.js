/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_common.js");

function getDistSpecifics () {
	var m = Number (getString ("m"));
	var n = Number (getString ("n"));
	return initDistSpecifics (i18n ('Wilcoxon Rank Sum distribution'), 'wilcox', ["m", "n"], [0, m*n], discrete);
}
