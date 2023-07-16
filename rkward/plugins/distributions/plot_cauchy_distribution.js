/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', location=' + getString ("loc") + ', scale=' + getString ("scale");
	getContRangeParameters ();

	options['distname'] = i18nc ("Cauchy distribution", noquote ("Cauchy"));
	if (options['is_density']) {
		options['fun'] = "dcauchy";
	} else {
		options['fun'] = "pcauchy";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("loc");
	header.addFromUI ("scale");
	return header;
}
