function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var data          = getValue("x");
	var chk_select    = getValue("chk_select");
	var inp_items     = getValue("inp_items");
	// reformat inp_items
		if (inp_items)
			inp_items = inp_items.split("\n").join(", ").replace(/(\w*)\[\["(\w*)"\]\]/g, '"$2"');
	var constraint    = getValue("constraint");
	var startval      = getValue("startval");
	var startval_lst  = getValue("startval_lst");
	var naaction      = getValue("naaction");
	var irtparam      = getValue("irtparam");
	var optimeth      = getValue("optimeth");
	var verbose       = getValue("verbose");
	// these are gpcm specific
	var ghk_gpcm      = getValue("ghk_gpcm");
	var iterqn_gpcm   = getValue("iterqn_gpcm");
	var optimizer     = getValue("optimizer");
	var numrderiv     = getValue("numrderiv");
	var epshess       = getValue("epshess");
	// parscale     = getValue("parscale"); not implemented yet...

	///////////////////////////////////
	// check for selected advanced control options
	var control = new Array() ;
	if (iterqn_gpcm != 150)
		control[control.length] = "iter.qN="+iterqn_gpcm ;
	if (ghk_gpcm != 21)
		control[control.length] = "GHk="+ghk_gpcm ;
	if (optimizer != "optim")
		control[control.length] = "optimizer=\"nlminb\"" ;
	if (optimizer == "optim" && optimeth != "BFGS")
		control[control.length] = "optimMethod=\""+optimeth+"\"" ;
	if (numrderiv != "fd")
		control[control.length] = "numrDeriv=\"cd\"" ;
	if (epshess != 1e-06)
		control[control.length] = "epsHes="+epshess ;
	if (verbose == "TRUE")
		control[control.length] = "verbose=TRUE" ;

	echo ('estimates.gpcm <- gpcm(');
		if (data && chk_select && inp_items)
			echo ("subset("+data+", select=c("+inp_items+"))");
		else
			echo (data);
		// any additional options?
		if (constraint != "gpcm") echo(", constraint=\""+constraint+"\"");
		if (irtparam != "TRUE") echo(", IRT.param=FALSE");
		if (startval == "list" && startval_lst) echo(", start.val="+startval_lst);
		if (startval == "random") echo(", start.val=\"random\"");
		if (naaction) echo(", na.action="+naaction);
		// finally check if any advanced control options must be inserted
		if (control.length > 0) echo(", control=list("+control.join(", ")+")");
	echo (')\n');
}

function printout () {
	// check whether parameter estimations should be kept in the global enviroment
	var save = getValue("chk_save");
	var save_name = getValue("save_name");

	echo ('rk.header ("GPCM parameter estimation")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(estimates.gpcm$call, width.cutoff=500))\n');
	echo ('rk.header ("Coefficients:", level=4)\n');
	echo ('rk.print (coef(estimates.gpcm))\n');
	echo ('rk.print (paste("Log-likelihood value at convergence:",round(estimates.gpcm$log.Lik, digits=1)))\n');
// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo (save_name + ' <<- estimates.gpcm\n');
	}
}
