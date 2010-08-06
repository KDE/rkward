function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var spin_samples    = getValue("spin_samples");

	echo ('unidim.res <- unidimTest(' + getValue("x"));
		// check if any options must be inserted
		if (spin_samples != 100) echo(", B=" + spin_samples) ;
	echo (')\n');
}

function printout () {
	var save         = getValue("save_name.active");
	var save_name    = getValue("save_name");

	echo ('rk.header ("Unidimensionality check (' + getValue("x") + ')")\n');
	echo ('rk.print ("Call:")\n');
	echo ('rk.print.literal (deparse(unidim.res$call, width.cutoff=500))\n');
	echo ('rk.header ("Matrix of tertachoric correlations:", level=4)\n');
	echo ('rk.print (unidim.res$Rho)\n');
	echo ('rk.header ("Unidimensionality Check using Modified Parallel Analysis:", level=4)\n');
	echo ('rk.print ("Alternative hypothesis: <em>The second eigenvalue of the observed data is substantially larger than the second eigenvalue of data under the assumed IRT model</em>")\n');
	echo ('rk.print (paste("Second eigenvalue in the observed data:", round(unidim.res$Tobs[2], digits=3)))\n');
	echo ('rk.print (paste("Average of second eigenvalues in Monte Carlo samples:", round(mean(unidim.res$T.boot[,2]), digits=3)))\n');
	echo ('rk.print (paste("Monte Carlo samples:", dim(unidim.res$T.boot)[1]))\n');
	echo ('rk.print (paste("p-value:", round(unidim.res$p.value, digits=3)))\n');
	// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo ('.GlobalEnv$' + save_name + ' <- unidim.res\n');
	}
}
