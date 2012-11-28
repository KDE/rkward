var prop_row, prop_column, prop_total, chisq, chisq_expetec, any_table_additions;

function preprocess () {
	prop_row = getValue ("prop_row") == "TRUE";
	prop_column = getValue ("prop_column") == "TRUE";
	prop_total = getValue ("prop_total") == "TRUE";
	chisq = getValue ("chisq") == "TRUE";
	chisq_expected = (getValue ("chisq_expected") == "TRUE") && chisq;
	any_table_additions = (prop_row || prop_column || prop_total || chisq_expected);
	if (!any_table_additions) return;

	echo ('# convenience function to bind together several two dimensional tables into a single three dimensional table\n');
	echo ('bind.tables <- function (...) {\n');
	echo ('	tables <- list (...)\n');
	echo ('	output <- unlist (tables)\n');
	echo ('	dim (output) <- c (dim (tables[[1]]), length (tables))\n');
	echo ('	dimnames (output) <- c (dimnames (tables[[1]]), list (statistic=names(tables)))\n');
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
	echo ('# calculate crosstabs\n');
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
			echo ('	results[[i]] <- bind.tables ("count"=addmargins (count)');
			if (prop_row) echo (',\n		"% of row"=addmargins (prop.table(count, 1) * 100, quiet=TRUE, FUN=function(x) NA)');
			if (prop_column) echo (',\n		"% of column"=addmargins (prop.table(count, 2) * 100, quiet=TRUE, FUN=function(x) NA)');
			if (prop_total) echo (',\n		"% of total"=addmargins (prop.table(count) * 100)');
			if (chisq_expected) echo (',\n		"expected"=addmargins (chisquares[[i]]$expected, quiet=TRUE, FUN=function(x) NA)');
			echo (')\n');
		} else {
			echo ('	results[[i]] <- bind.tables ("count"=count');
			if (prop_row) echo (',\n		"% of row"=prop.table(count, 1) * 100');
			if (prop_column) echo (',\n		"% of column"=prop.table(count, 2) * 100');
			if (prop_total) echo (',\n		"% of total"=prop.table(count) * 100');
			if (chisq_expected) echo (',\n		"expected"=chisquares[[i]]$expected');
			echo (')\n');
		}
	}
	echo ('}\n');
}

function printout () {
	doPrintout (true);
}

function preview () {
	calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	if (full) {
		echo ('rk.header ("Crosstabs (n to 1)", level=1)\n');
		echo ('for (i in 1:length (results)) {\n');
		echo ('	rk.header ("Crosstabs (n to 1)", parameters=list ("Dependent", names (x)[1], "Independent", names (yvars)[i]), level=2)\n');
		if (any_table_additions) {
			echo ('	rk.print (ftable (results[[i]], col.vars=2))\n');
		} else {
			echo ('	rk.results (results[[i]], titles=c(names (x)[1], names (yvars)[i]))\n');
		}
		if (getValue ("chisq") == "TRUE") {
			echo ('\n');
			echo ('	rk.header ("Pearson\'s Chi Square Test for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "Method", chisquares[[i]][["method"]]), level=2)\n');
			echo ('	rk.results (list (\'Statistic\'=chisquares[[i]][[\'statistic\']], \'df\'=chisquares[[i]][[\'parameter\']], \'p\'=chisquares[[i]][[\'p.value\']]))\n');
		}

		if (getValue ("barplot") == "TRUE") {
			echo ('\n');
			echo ('	rk.header ("Barplot for Crosstabs", list ("Dependent"=names (x)[1], "Independent"=names (yvars)[i]' + getValue ('barplot_embed.code.preprocess') + '), level=2)\n');
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
	} else {
		// produce a single barplot of the first result
		if (any_table_additions) {
			echo ('counts <- results[[1]][, , "count"]\n');
		} else {
			echo ('counts <- results[[1]]\n');
		}
		echo (getValue ('barplot_embed.code.printout'));
	}
}

