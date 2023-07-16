/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', df=' + getString ("df") + ', ncp=' + getString ("ncp");
	getContRangeParameters ();

	options['distname'] = i18nc ("t distribution", noquote ("Student t"));
	if (options['is_density']) {
		options['fun'] = "dt";
	} else {
		options['fun'] = "pt";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df");
	header.addFromUI ("ncp");
	return header;
}
