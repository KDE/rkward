function preprocess () {
	// we'll need the eRm package, so in case it's not loaded...
	echo ('require(eRm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var design       = getValue("design");
	var design_mtx   = getValue("design_mtx");
	var etastart     = getValue("etastart");
	var etastart_vec = getValue("etastart_vec");
	var stderr       = getValue("stderr");
	var sumnull      = getValue("sumnull");

	echo ('estimates.pcm <- PCM(' + getValue("x"));
		// any additional options?
		if (design == "matrix") echo(", W="+design_mtx);
		if (stderr != "se") echo(", se=FALSE");
		if (sumnull != "sum0") echo(", sum0=FALSE");
		if (etastart == "startval") echo(", etaStart="+etastart_vec);
	echo (')\n');
}

function printout () {
	// check whether parameter estimations should be kept in the global enviroment
	var save         = getValue("chk_save");
	var save_name    = getValue("save_name");

	echo ('rk.header ("PCM  parameter estimation")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(estimates.pcm$call, width.cutoff=500))\n');
	echo ('rk.header ("Coefficients:", level=4)\n');
	echo ('rk.print(t(rbind(Eta=estimates.pcm$etapar,StdErr=estimates.pcm$se.eta)))\n');
	echo ('rk.print (paste("Conditional log-likelihood:",round(estimates.pcm$loglik, digits=1),\n');
	echo ('"<br />Number of iterations:",estimates.pcm$iter,"<br />Number of parameters:",estimates.pcm$npar))\n');
// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo (save_name + ' <<- estimates.pcm\n');
	}
}
