/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', df1=' + getString ("df1") + ', df2=' + getString ("df2") + ', ncp=' + getString ("ncp");
	getContRangeParameters ();

	options['distname'] = i18nc ("F distribution", noquote ("F"));
	if (options['is_density']) {
		options['fun'] = "df";
	} else {
		options['fun'] = "pf";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df1");
	header.addFromUI ("df2");
	header.addFromUI ("ncp");
	return header;
}
