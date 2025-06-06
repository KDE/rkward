/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preview() {
	doPrintout(false);
}

function printout() {
	doPrintout(true);
}

function doPrintout(full) {
	var grouped_mode = getValue("data_mode_grouped.numeric");
	var dodged = grouped_mode && getValue("dodges");
	var names_mode = getValue("names_mode");
	var do_mean = getValue("mean.checked");
	var pch_mean = getValue("pch_mean");
	var do_sd = getValue("sd.checked");
	var pch_sd_high = getValue("pch_sd_high");
	var pch_sd_low = getValue("pch_sd_low");
	var horizontal = getValue("orientation") == "TRUE";
	var boxwex = getValue("boxwex");
	if (!dodged && (Number(boxwex) != .8)) boxwex = ", boxwex=" + boxwex;
	else boxwex = "";
	var positions = "";

	if (grouped_mode) {
		var groups_a = getValue("groups").split("\n");
		var n_dodges = getValue("dodges");
		var dodges = groups_a.slice(0, n_dodges);
		var groups = groups_a.slice(n_dodges);
		echo('groups <- rk.list (' + dodges.concat(groups).join(", ") + ')\n');
		echo('data_list <- split (' + getValue("outcome") + ', groups)		');
		comment('split sample by grouping variables');
		if (dodged) {
			comment('adjust width and position of boxes to achieve dodging');
			echo('dodge_size <- nlevels (interaction (' + dodges.join(", ") + '))\n');
			echo('box_width <- ' + getValue("boxwex") + ' / dodge_size\n');
			boxwex = ', boxwex=box_width';
			echo('box_positions <- (rep (1:(length (data_list) / dodge_size), each=dodge_size) + (1:dodge_size)*(box_width))\n')
			positions = ', at=box_positions, xlim=c(min(box_positions)-box_width, max(box_positions)+box_width)';
		}
	} else {
		echo('data_list <- rk.list (' + getValue("x").split("\n").join(", ") + ')		');
		comment('convert single sample variables to list');
	}
	if (names_mode == "rexp") {
		echo("names(data_list) <- " + getValue("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo("names(data_list) <- c (\"" + str_replace(";", "\", \"", trim(getValue("names_custom"))) + "\")\n");
	}

	if (full) {
		if (grouped_mode) {
			new Header(i18n("Boxplot")).add(i18n("Outcome variable"), noquote('rk.get.description (' + getValue("outcome") + ')')).add(i18n("Grouping variable(s)"), noquote("paste (names (groups), collapse=\", \")")).print();
		} else {
			new Header(i18n("Boxplot")).add(i18n("Variable(s)"), noquote("paste (names (data_list), collapse=\", \")")).print();
		}
		echo('rk.graph.on()\n');
	}

	echo('try ({\n');
	printIndentedUnlessEmpty("\t", getValue("plotoptions.code.preprocess"), '', '\n');

	echo('	boxplot (data_list' + boxwex + positions + ', notch = ' + getValue("notch") + ', outline = ' + getValue("outline") + ', horizontal = ' + getValue("orientation") + getValue("plotoptions.code.printout") + ') ');
	comment("actual boxplot function");
	if (do_mean) {
		var mean_fun = "mean, na.rm=TRUE";
		if (getValue("type_of_mean") == "geometric_mean") {
			echo('	geo_mean <- function (x) {prod(na.omit(x))^(1/length(na.omit(x)))}	');
			comment('Calculate geometric mean');
			mean_fun = "geo_mean";
		} else if (getValue("type_of_mean") == "harmonic_mean") {
			echo('	har_mean <- function (x) {(1 / mean(1 / na.omit(x)))}	');
			comment('Calculate harmonic mean');
			mean_fun = "har_mean";
		} else if (getValue("type_of_mean") == "interquartile_mean") {
			echo('	interq_mean <- function (x) {sum(quantile(x, probs=c(0.25), na.rm=TRUE), quantile(x, probs=c(0.75), na.rm=TRUE)) / 2}	');
			comment('Calculate the interquartile mean');
			mean_fun = "interq_mean";
		} else {                          // arithmetic mean
			var trimp = getValue("trim"); // NOTE: avoid name clash with utility function trim()!
			if (trimp != 0) mean_fun += ", trim=" + trimp;
		}

		if (horizontal) {
			echo('	points(1:length(data_list) ~ sapply(data_list, ' + mean_fun + '), pch=' + pch_mean + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ') ');
			comment('calculates the mean for all data and adds a point at the corresponding position');
		} else {
			echo('	points(sapply(data_list, ' + mean_fun + '), pch=' + pch_mean + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ') ');
			comment('calculates the mean for all data and adds a point at the corresponding position');
		}
	}

	if (do_sd) {
		echo('	sd_low <- (sapply(data_list, mean, na.rm = TRUE)) - (sapply(data_list,sd,na.rm = TRUE))\n');
		echo('	sd_high <- (sapply(data_list, mean, na.rm = TRUE)) + (sapply(data_list,sd,na.rm = TRUE))\n');
		if (horizontal) {
			echo('	points(1:length(data_list) ~ sd_low, pch=' + pch_sd_low + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ')\n');
			echo('	points(1:length(data_list) ~ sd_high, pch=' + pch_sd_high + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ')\n');
		} else {
			echo('	points(sd_low, pch=' + pch_sd_low + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ')\n');
			echo('	points(sd_high, pch=' + pch_sd_high + ', cex = ' + getValue("cex_sd_mean") + getValue("sd_mean_color.code.printout") + ')\n');
		}
	}

	printIndentedUnlessEmpty("\t", getValue("plotoptions.code.calculate"), '\n');
	echo('})\n'); // end of try ()

	if (full) {
		echo('rk.graph.off ()\n');
	}
}
