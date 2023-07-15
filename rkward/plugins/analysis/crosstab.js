/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
var prop_row, prop_column, prop_total, chisq, chisq_expetec, any_table_additions;

function preprocess () {
	prop_row = getValue ("prop_row") == "TRUE";
	prop_column = getValue ("prop_column") == "TRUE";
	prop_total = getValue ("prop_total") == "TRUE";
	chisq = getValue ("chisq") == "TRUE";
	chisq_expected = (getValue ("chisq_expected") == "TRUE") && chisq;
	any_table_additions = (prop_row || prop_column || prop_total || chisq_expected);
	if (!any_table_additions) return;

	comment ('convenience function to bind together several two dimensional tables into a single three dimensional table\n');
	echo ('bind.tables <- function (...) {\n');
	echo ('	tables <- list (...)\n');
	echo ('	output <- unlist (tables)\n');
	echo ('	dim (output) <- c (dim (tables[[1]]), length (tables))\n');
	echo ('	dimnames (output) <- c (dimnames (tables[[1]]), list (' + i18nc ("a statistic indicator" ,"statistic") + '=names(tables)))\n');
	echo ('	output\n');
	echo ('}\n');
}

function calculate () {
	var x = getValue ("x") ;
	var y = getList ("y").join (', ');
	var margins = (getValue ("margins") == "TRUE");

	echo ('x <- rk.list (' + x + ')\n');
	echo ('yvars <- rk.list (' + y + ')\n');
	echo ('results <- list()\n');
	if (chisq) echo ('chisquares <- list ()\n');
	echo ('\n');
	comment ('calculate crosstabs\n');
	echo ('for (i in 1:length (yvars)) {\n');
	echo ('	count <- table(x[[1]], yvars[[i]])\n');
	if (chisq) {
		echo ('	chisquares[[i]] <- chisq.test (count, simulate.p.value = ' + getValue ("simpv"));
		if (getValue ("simpv") == "TRUE") {
			echo (',B=(');
			echo (getValue ("B"));
			echo (') ');
		}
		echo (')\n');
	}
	if (!any_table_additions) {
		if (margins) echo ('	results[[i]] <- addmargins (count)\n');
		else echo ('	results[[i]] <- count\n');
	} else {
		// unfortunately, mixing margins and proportions is a pain, in that they don't make a whole lot of sense for "% of row", and "% of column"
		if (margins) {
			echo ('	results[[i]] <- bind.tables (' + i18nc ("noun", "count") + '=addmargins (count)');
			if (prop_row) echo (',\n		' + i18n ("% of row") + '=addmargins (prop.table(count, 1) * 100, quiet=TRUE, FUN=function(x) NA)');
			if (prop_column) echo (',\n		' + i18n ("% of column") + '=addmargins (prop.table(count, 2) * 100, quiet=TRUE, FUN=function(x) NA)');
			if (prop_total) echo (',\n		' + i18n ("% of total") + '=addmargins (prop.table(count) * 100)');
			if (chisq_expected) echo (',\n		' + i18nc ("expected count", "expected") + '=addmargins (chisquares[[i]]$expected, quiet=TRUE, FUN=function(x) NA)');
			echo (')\n');
		} else {
			echo ('	results[[i]] <- bind.tables (' + i18nc ("noun", "count") + '=count');
			if (prop_row) echo (',\n		' + i18n ("% of row") + '=prop.table(count, 1) * 100');
			if (prop_column) echo (',\n		' + i18n ("% of column") + '=prop.table(count, 2) * 100');
			if (prop_total) echo (',\n		' + i18n ("% of total") + '=prop.table(count) * 100');
			if (chisq_expected) echo (',\n		' + i18nc ("expected count", "expected") + '=chisquares[[i]]$expected');
			echo (')\n');
		}
	}
	echo ('}\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Crosstabs (n to 1)"), 1).print ();
		echo ('for (i in 1:length (results)) {\n');
	} else {
		echo ('for (i in 1:1) {\n');
	}
	sectionHeader (i18n ("Crosstabs (n to 1)"), "");
	if (any_table_additions) {
		echo ('	rk.print (ftable (results[[i]], col.vars=2))\n');
	} else {
		echo ('	rk.results (results[[i]], titles=c(names (x)[1], names (yvars)[i]))\n');
	}
	if (getValue ("chisq") == "TRUE") {
		echo ('\n');
		sectionHeader (i18n ("Pearson\'s Chi Square Test for Crosstabs"), ', ' + i18n ("Method") + '=chisquares[[i]][["method"]]');
		echo ('	rk.results (list (' + i18nc ("a statistic indicator", "Statistic") + '=chisquares[[i]][[\'statistic\']], \'df\'=chisquares[[i]][[\'parameter\']], \'p\'=chisquares[[i]][[\'p.value\']]))\n');
	}

	if (getValue ("barplot") == "TRUE") {
		echo ('\n');
		sectionHeader (i18n ("Barplot for Crosstabs"), getValue ('barplot_embed.code.preprocess'));
		echo ('	rk.graph.on ()\n');
		echo ('	try ({\n');
		if (any_table_additions) {
			echo ('		counts <- results[[i]][, , "count"]\n');
		} else {
			echo ('		counts <- results[[i]]\n');
		}
		printIndented ("\t\t", getValue ('barplot_embed.code.printout'));
		echo ('	})\n');
		echo ('	rk.graph.off ()\n');
	}
	echo ('}\n');
}

function sectionHeader (title, additions) {
	echo ('\trk.header (' + quote (title) + ', parameters=list (' + i18nc ("dependent variable", "Dependent") + '=names (x)[1], '
	                      + i18nc ("independent variable", "Independent") + '=names (yvars)[i]' + additions + '), level=2)\n');
}
