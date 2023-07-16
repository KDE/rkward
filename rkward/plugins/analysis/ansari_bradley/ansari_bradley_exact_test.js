/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(exactRankTests)\n');
	echo ('\n');
	echo ('names <- rk.get.description (' + getValue ("x") + ', ' + getValue ("y") + ')\n');
}

function calculate () {
	var exact_opt = "";
	var exact_setting = getValue ("exact");
	if (exact_setting == "yes") {
		exact_opt = ", exact=TRUE";
	} else if (exact_setting == "no") {
		exact_opt = ", exact=FALSE";
	}

	var conflevel_opt = "";
	if (getValue ("confint") == "TRUE") {
		var conflevel = getValue("conflevel");
		if (conflevel != "0.95") conflevel_opt = ", conf.level=" + conflevel;
	}

	echo ('result <- ansari.exact (' + getValue ("x") + ', ' + getValue ("y") + ', alternative = "' + getValue ("alternative") + '"' + exact_opt + ', conf.int = ' + getValue ("confint") + conflevel_opt + ')\n');
	echo ('\n');
}

function printout (is_preview) {
	if (!is_preview) {
		var header = new Header (noquote ('result$method'));
		header.addFromUI ("x", noquote ('names[1]'));
		header.addFromUI ("y", noquote ('names[2]'));
		header.add (i18n ("H1"), noquote ('rk.describe.alternative(result)'));
		header.addFromUI ("exact");
		if (getBoolean ("confint.state")) {
			header.addFromUI ("conflevel");
		}
		header.print ();
		echo ('\n');
	}
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variable Names") + '=names,\n');
	echo ('	' + i18n ("statistic") + '=result$statistic,\n');
	echo ('	' + i18n ("null.value") + '=result$null.value,\n');
	echo ('	p=result$p.value');
	if (getBoolean ("confint.state")) {
		echo (',\n');
		echo ('	' + i18n ("confidence interval percent") + '=(100 * attr(result$conf.int, "conf.level")),\n');
		echo ('	' + i18n ("confidence interval of difference") + '=result$conf.int,\n');
		echo ('	' + i18n ("estimate of the ratio of scales") + '=result$estimate');
	}
	echo ('))\n');
}

