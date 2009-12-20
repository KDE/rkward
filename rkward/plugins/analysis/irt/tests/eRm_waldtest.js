function preprocess () {
	// we'll need the eRm package, so in case it's not loaded...
	echo ('require(eRm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var rad_splitcr    = getValue("rad_splitcr");
	var splitvector    = getValue("splitvector");

	echo ('waldtest.res <- Waldtest(' + getValue("x"));
		// check if any advanced control options must be inserted
		if (rad_splitcr == "mean") echo(", splitcr=\"mean\"") ;
		if (rad_splitcr == "vector" && splitvector) echo(", splitcr="+splitvector) ;
	echo (')\n');
}

function printout () {
	echo ('rk.header ("Wald test (' + getValue("x") + ')")\n');
	echo ('rk.print (waldtest.res)\n');
}
