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
	// these are 3pl specific
	var ghk_3pl      = getValue("ghk_3pl");
	var iterqn_3pl   = getValue("iterqn_3pl");
	var type         = getValue("type");
	var maxguess     = getValue("maxguess");
	var optimizer    = getValue("optimizer");
	var epshess      = getValue("epshess");
	// var parscale     = getValue("parscale"); not implemented yet...

	///////////////////////////////////
	// check for selected advanced control options
	var control = new Array() ;
	if (optimizer != "optim")
		control[control.length] = "optimizer=\"nlminb\"" ;
	if (iterqn_3pl != 1000)
		control[control.length] = "iter.qN="+iterqn_3pl ;
	if (ghk_3pl != 21)
		control[control.length] = "GHk="+ghk_3pl ;
	if (optimizer == "optim" && optimeth != "BFGS")
		control[control.length] = "method=\""+optimeth+"\"" ;
	if (verbose == "TRUE")
		control[control.length] = "verbose=TRUE" ;
	if (epshess != 1e-03)
		control[control.length] = "eps.hessian="+epshess ;

	echo ('estimates.3pl <- tpm(' + getValue("x"));
		// any additional options?
		if (type == "rasch") echo(", type=\"rasch\"");
		if (constraint) echo(", constraint="+constraint);
		if (maxguess != 1) echo(", max.guessing="+maxguess);
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
	var save = getValue("save_name.active");
	var save_name = getValue("save_name");

	echo ('rk.header ("3PL parameter estimation")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(estimates.3pl$call, width.cutoff=500))\n');
	echo ('rk.header ("Coefficients:", level=4)\n');
	echo ('rk.print (coef(estimates.3pl))\n');
	echo ('rk.print (paste("Log-likelihood value at convergence:",round(estimates.3pl$log.Lik, digits=1)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo ('.GlobalEnv$' + save_name + ' <- estimates.3pl\n');
	}
}
