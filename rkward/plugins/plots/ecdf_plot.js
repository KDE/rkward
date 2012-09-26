function preprocess () {
	var x = getValue ("x");

	echo ('yrange <- range (' + x + ', na.rm=TRUE)\n');
	if (getValue ("th_pnorm") && getValue ("adjust_th_pnorm")) {
		echo ('data.mean <- mean (' + x + ', na.rm=TRUE)\n');
		echo ('data.sd <- sd (' + x + ', na.rm=TRUE)\n');
	}
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var x = getValue ("x");

	if (full) {
		echo ('rk.header ("Empirical Cumulative Distribution Function", list ("Variable", rk.get.description (' + x + '), "Minimum", yrange[1], "Maximum", yrange[2]))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	plot.ecdf (' + x + ', ' + getValue ("stepfun_options.code.printout") + getValue ("plotoptions.code.printout") + ')\n');
	if (getValue ("th_pnorm")) {
		echo ('	curve (pnorm');
		if (getValue ("adjust_th_pnorm")) echo (" (x, mean=data.mean, sd=data.sd)");
		echo (', from=yrange[1], to=yrange[2], add=TRUE, ' + getValue ("col_thnorm.code.printout") + ')\n');
	}
	if (getValue ("rug")) {
		echo ('	rug (' + x + ', ' + getValue ("ticksize") + ', ' + getValue ("lwd") + ', ' + getValue ("side") + getValue ("col_rug.code.printout") + ')\n');
	}

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}


