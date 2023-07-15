/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("dist_common.js");

function getDistSpecifics () {
	var size = Number (getString ("size"));
	var prob = Number (getString ("prob"));
	return initDistSpecifics (i18n ('Negative Binomial distribution'), 'nbinom', ["size", "prob"], [0, Math.ceil (size * 1.5 / prob)], discrete);
}
