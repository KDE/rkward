/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', lambda=' + getString ("mean");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Poisson distribution", noquote ("Poisson"));
	if (options['is_density']) {
		options['fun'] = "dpois";
	} else {
		options['fun'] = "ppois";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("mean");
	return header;
}
