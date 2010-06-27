function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var constraint   = getValue("constraint");
	var startval     = getValue("startval");
	var startval_mtx = getValue("startval_mtx");
	var naaction     = getValue("naaction");
	var irtparam     = getValue("irtparam");
	var optimeth     = getValue("optimeth");
	var verbose      = getValue("verbose");
	// these are rasch specific
	var ghk_rasch = getValue("ghk_rasch");
	var iterqn_rasch = getValue("iterqn_rasch");

	///////////////////////////////////
	// check for selected advanced control options
	var control = new Array() ;
	if (iterqn_rasch != 150)
		control[control.length] = "iter.qN="+iterqn_rasch ;
	if (ghk_rasch != 21)
		control[control.length] = "GHk="+ghk_rasch ;
	if (optimeth != "BFGS")
		control[control.length] = "method=\""+optimeth+"\"" ;
	if (verbose == "TRUE")
		control[control.length] = "verbose=TRUE" ;

	echo ('estimates.rasch <- rasch(' + getValue("x"));
		// any additional options?
		if (constraint) echo(", constraint="+constraint);
		if (irtparam != "TRUE") echo(", IRT.param=FALSE");
		if (startval == "random") echo(", start.val=\"random\"");
		if (startval == "matrix") echo(", start.val="+startval_mtx);
		if (naaction) echo(", na.action="+naaction);
		// finally check if any advanced control options must be inserted
		if (control.length > 0) echo(", control=list("+control.join(", ")+")");
	echo (')\n');
}

function printout () {
	// check whether parameter estimations should be kept in the global enviroment
	var save      = getValue("save_name.active");
	var save_name = getValue("save_name");
	var irtparam  = getValue("irtparam");

	echo ('rk.header ("Rasch parameter estimation")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(estimates.rasch$call, width.cutoff=500))\n');
	echo ('rk.header ("Coefficients:", level=4)\n');
	echo ('rk.print (coef(estimates.rasch))\n');
	echo ('rk.print (paste("Log-likelihood value at convergence:",round(estimates.rasch$log.Lik, digits=1)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo ('.GlobalEnv$' + save_name + ' <- estimates.rasch\n');
	}
}
