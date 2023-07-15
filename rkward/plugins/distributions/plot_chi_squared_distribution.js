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

	options['distname'] = i18nc ("Chi-square distribution", noquote ("Chi-square"));
	if (options['is_density']) {
		options['fun'] = "dchisq";
	} else {
		options['fun'] = "pchisq";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df");
	header.addFromUI ("ncp");
	return header;
}
