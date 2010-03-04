// globals
var nAvg;
var nDist;

function printout () {
	doPrintout (true);
}

function preview () {
	if (typeof (preprocess) != "undefined") preprocess ();
	if (typeof (calculate) != "undefined") calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	var fun = getValue ("function");
	nAvg = getValue ("nAvg"); // number of observations to calculate the averages
	nDist = getValue ("nDist"); // number of sample to construct the distribution

	var scalenorm = getValue ("scalenorm"); // if variables should to normalised..
	var drawnorm = getValue ("drawnorm");


	echo ('# parameters:\n');
	doParameters (); // get the parameters from xml file and store them R varaibles

	if (scalenorm || drawnorm) {
		echo ('# mean and variances of the distribution of sample averages:\n');
		doExpVar (); // calculate the expectation and varaince of the distribution of smaple averages
	}

	// Mean and Std.deviantion of Normal distribution:
	var normMuSigma_tag = ""; // defaults to mean=0, sd=1.
	if (!scalenorm) normMuSigma_tag = ", mean = avg.exp, sd = sqrt(avg.var)";

	var plotpre = getValue ("plotoptions.code.preprocess");
	var plotoptions = getValue ("plotoptions.code.printout");
	var plotadds = getValue ("plotoptions.code.calculate");
	var normFun = "";
	var histcalcoptions = "";
	var histplotoptions = "";
	if (fun == "hist") {
		normFun = "dnorm"; // draw normal density on the histogram
		histcalcoptions = getValue ("histogram_opt.code.calculate"); // options that goes into hist() function
		histplotoptions = getValue ("histogram_opt.code.printout"); // options that goes into plot.histogram()
		histplotoptions += plotoptions; // generic plot options
	} else if (fun == "dist") {
		normFun = "pnorm"; // draw normal cdf on the ecdf plot
		plotoptions += getValue ("dist_stepfun.code.printout"); // plot.ecdf() and plot.stepfun() options
	}

	var yLim = ""; // initialise the ylim option


	echo ('# generate the entire data:\n');
	doGenerateData (); // generate the random samples


	echo ('# get the sample averages:\n');
	echo ('avg <- colMeans(data);\n');

	if (scalenorm) {
		echo ('# normalise the variables:\n');
		echo ('avg <- (avg - avg.exp)/sqrt(avg.var);\n');
	}
	if (drawnorm) {
		echo ('# generate random normal samples:\n');
		echo ('normX <- seq(from=min(avg), to=max(avg), length=' + nDist + ');\n');
		echo ('normY <- ' + normFun + ' (normX' + normMuSigma_tag + ');\n');
	}
	if (fun == "hist") {
		echo ('dist.hist <- hist(avg, plot=FALSE' + histcalcoptions + ');\n');
		if (drawnorm) {
			echo ('# calculate the ylims appropriately:\n');
			echo ('ylim <- c(0,max(c(dist.hist$density, normY)));\n');
			yLim = ', ylim=ylim';
		}
	}

	if (full) {
		echo ('rk.graph.on ()\n');
		echo ('try ({\n');
	}

	if (plotpre.length > 0) printIndented ("\t", plotpre);
	if (fun == "hist") {
		echo ('	plot(dist.hist' + yLim + histplotoptions + ')\n');
	} else if (fun == "dist") {
		echo ('	plot(ecdf(avg)' + plotoptions + ')\n');
	}
	if (drawnorm) {
		echo ('	lines (x=normX, y=normY, type="' + getValue ("normpointtype") + '"' + getValue ("normlinecol.code.printout") + ')\n');
	}
	if (plotadds.length > 0) printIndented ("\t", plotadds);

	if (full) {
		echo ('})\n');
		echo ('rk.graph.off ()\n');
	}
}
