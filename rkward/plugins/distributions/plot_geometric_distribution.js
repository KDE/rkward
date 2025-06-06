/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

include('plot_dist_common.js');

function getParameters() {
	options['args'] = ', prob=' + getString("prob");
	getDiscontRangeParameters();

	options['distname'] = i18nc("Geometric distribution", noquote("Geometric"));
	if (options['is_density']) {
		options['fun'] = "dgeom";
	} else {
		options['fun'] = "pgeom";
	}
}

function addParamsToHeader(header) {
	header.addFromUI("prob");
	return header;
}
