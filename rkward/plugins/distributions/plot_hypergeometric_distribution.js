/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', m=' + getString ("m") + ', n=' + getString ("n_val") + ', k=' + getString ("k");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Hypergeometric distribution", noquote ("Hypergeometric"));
	if (options['is_density']) {
		options['fun'] = "dhyper";
	} else {
		options['fun'] = "phyper";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("m");
	header.addFromUI ("n_val");
	header.addFromUI ("k");
	return header;
}
