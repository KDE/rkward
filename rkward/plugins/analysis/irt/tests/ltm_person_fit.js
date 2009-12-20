function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var rad_hypot       = getValue("rad_hypot");
	var rad_resppat     = getValue("rad_resppat");
	var mtx_resppat     = getValue("mtx_resppat");
	var rad_pvalue      = getValue("rad_pvalue");
	var spin_mc         = getValue("spin_mc");

	///////////////////////////////////
	// check for selected options
	var options = new Array() ;
	if (rad_hypot == "greater" || rad_hypot == "two.sided")
		options[options.length] = "alternative=\""+rad_hypot+"\"" ;
	if (rad_resppat == "resp_matrix" && mtx_resppat)
		options[options.length] = "resp.patterns="+mtx_resppat ;
	if (rad_pvalue == "montecarlo")
		options[options.length] = "simulate.p.value=TRUE" ;
	if (rad_pvalue == "montecarlo" && spin_mc != 1000 )
		options[options.length] = "B="+spin_mc ;


	echo ('personfit.res <- person.fit(' + getValue("x"));
		// check if any advanced control options must be inserted
		if (options.length > 0) echo(", "+options.join(", "));
	echo (')\n');
}

function printout () {
	echo ('rk.header ("Person-fit statistics (' + getValue("x") + ')")\n');
	echo ('rk.print (personfit.res)\n');
}
