function calculate () {
	var x = getValue ("x") ;
	var y = trim (getValue ("y")).split (/\n/).join (', ');

	echo ('x <- rk.list (' + x + ')\n');
	echo ('yvars <- rk.list (' + y + ')\n');
	echo ('results <- list()\n');
	echo ('\n');
	echo ('# calculate crosstabs\n');
	echo ('for (i in 1:length (yvars)) {\n');
	echo ('	results[[i]] <- table(x[[1]], yvars[[i]])\n');
	echo ('}\n');
	if (getValue ("chisq") == "TRUE") {
		echo ('\n');
		echo ('# calculate chisquares\n');
		echo ('chisquares <- list ()\n');
		echo ('for (i in 1:length (results)) {\n');
		echo ('	chisquares[[i]] <- chisq.test (results[[i]], simulate.p.value = ' + getValue ("simpv"));
		if (getValue ("simpv") == "TRUE") {
			echo (',B=(');
			echo (getValue ("B"));
			echo (') ');
		}
		echo (')\n');
		echo ('}\n');
	}

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
		echo ('	rk.results (results[[i]], titles=c(names (x)[1], names (yvars)[i]))\n');
		if (getValue ("chisq") == "TRUE") {
			echo ('\n');
			echo ('	rk.header ("Pearson\'s Chi Square Test for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "Method", chisquares[[i]][["method"]]), level=2)\n');
			echo ('	rk.results (list (\'Statistic\'=chisquares[[i]][[\'statistic\']], \'df\'=chisquares[[i]][[\'parameter\']], \'p\'=chisquares[[i]][[\'p.value\']]))\n');
		}

		if (getValue ("barplot") == "TRUE") {
			echo ('\n');
			echo ('	rk.header ("Barplot for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i]' + getValue ('barplot_embed.code.preprocess') + '), level=2)\n');
			echo ('	rk.graph.on ()\n');
			echo ('	try ({\n');
			printIndented ("\t\t", getValue ('barplot_embed.code.printout'));
			echo ('	})\n');
			echo ('	rk.graph.off ()\n');
		}
		echo ('}\n');
	} else {
		// produce a single barplot of the first result
		echo ("i <- 1\n");
		echo (getValue ('barplot_embed.code.printout'));
	}
}

