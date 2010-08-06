function preprocess () {
	// we'll need the eRm package, so in case it's not loaded...
	echo ('require(eRm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var rad_splitcr    = getValue("rad_splitcr");
	var splitvector    = getValue("splitvector");
	var optimizer      = getValue("drop_optimizer");

	if (optimizer != "nlm") echo('.GlobalEnv$fitctrl <- \"'+optimizer+'\"\n');
	echo ('waldtest.res <- Waldtest(' + getValue("x"));
		// check if any advanced control options must be inserted
		if (rad_splitcr == "mean") echo(", splitcr=\"mean\"");
		if (rad_splitcr == "vector" && splitvector) echo(", splitcr="+splitvector);
	echo (')\n');
	if (optimizer != "nlm") echo('rm(fitctrl, envir=.GlobalEnv)\n');
}

function printout () {
	echo ('rk.header ("Wald test (' + getValue("x") + ')")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(waldtest.res$call, width.cutoff=500))\n');
	echo ('rk.header ("Wald test on item level (z-values):", level=4)\n');
	echo ('rk.print(waldtest.res$coef.table)\n');
}
