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

	options['distname'] = i18nc ("Logistic distribution", noquote ("Logistic"));
	if (options['is_density']) {
		options['fun'] = "dlogis";
	} else {
		options['fun'] = "plogis";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("loc");
	header.addFromUI ("scale");
	return header;
}
