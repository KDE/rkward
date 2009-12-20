function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var spin_groups     = getValue("spin_groups");
	var drop_sumgroups  = getValue("drop_sumgroups");
	var rad_pvalue      = getValue("rad_pvalue");
	var spin_mc         = getValue("spin_mc");

	///////////////////////////////////
	// check for selected options
	var options = new Array() ;
	if (spin_groups != 10)
		options[options.length] = "G="+spin_groups ;
	if (drop_sumgroups != "median")
		options[options.length] = "FUN="+drop_sumgroups ;
	if (rad_pvalue == "montecarlo")
		options[options.length] = "simulate.p.value=TRUE" ;
	if (rad_pvalue == "montecarlo" && spin_mc != 100 )
		options[options.length] = "B="+spin_mc ;


	echo ("itemfit.res <- item.fit(" + getValue("x"));
		// check if any advanced control options must be inserted
		if (options.length > 0) echo(", "+options.join(", "));
	echo (')\n');
}

function printout () {
	echo ('rk.header ("Item-fit statistics (' + getValue("x") + ')")\n');
	echo ('rk.print (itemfit.res)\n');
}
