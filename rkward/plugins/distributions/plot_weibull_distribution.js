/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', shape=' + getString ("shape") + ', scale=' + getString ("scale");
	getContRangeParameters ();

	options['distname'] = i18nc ("Weibull distribution", noquote ("Weibull"));
	if (options['is_density']) {
		options['fun'] = "dweibull";
	} else {
		options['fun'] = "pweibull";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("shape");
	header.addFromUI ("scale");
	return header;
}
