/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', m=' + getString ("nm") + ', n=' + getString ("nn");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Wilcoxon Rank Sum distribution", noquote ("Wilcoxon Rank Sum"));
	if (options['is_density']) {
		options['fun'] = "dwilcox";
	} else {
		options['fun'] = "pwilcox";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("nm");
	header.addFromUI ("nn");
	return header;
}
