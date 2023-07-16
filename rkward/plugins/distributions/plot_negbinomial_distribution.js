/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	if (getValue ("param") == "pprob") {
		options['args'] = ', size=' + getString ("size_trial") + ', prob=' + getString ("prob");
		options['header_adds'] = ["size_trial", "prob"];
	} else {
		options['args'] = ', size=' + getString ("size_disp") + ', mu=' + getString ("mu");
		options['header_adds'] = ["size_disp", "mu"];
	}
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Negative Binomial distribution", noquote ("Negative Binomial"));
	if (options['is_density']) {
		options['fun'] = "dnbinom";
	} else {
		options['fun'] = "pnbinom";
	}
}

function addParamsToHeader (header) {
	for (var i = 0; i < options['header_adds'].length; ++i) {
		header.addFromUI (options['header_adds'][i]);
	}
	return header;
}
