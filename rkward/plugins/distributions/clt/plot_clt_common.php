<?php
function preprocess () {
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function doPrintout ($final) {
	global $nAvg;
	global $nDist;

	$fun = getRK_val ("function");
	$nAvg = getRK_val ("nAvg"); // number of observations to calculate the averages
	$nDist = getRK_val ("nDist"); // number of sample to construct the distribution

	$scalenorm = getRK_val ("scalenorm"); // if variables should to normalised..
	$drawnorm = getRK_val ("drawnorm");

?>
# parameters:
<?
	doParameters (); // get the parameters from xml file and store them R varaibles

	if ($scalenorm || $drawnorm) {
?>
# mean and variances of the distribution of sample averages:
<?
		doExpVar (); // calculate the expectation and varaince of the distribution of smaple averages
	}

	// Mean and Std.deviantion of Normal distribution:
	if ($scalenorm) $normMuSigma_tag = ""; // defaults to mean=0, sd=1.
	else $normMuSigma_tag = ", mean = avg.exp, sd = sqrt(avg.var)";

	$plotpre = getRK_val ("plotoptions.code.preprocess");
	$plotoptions = getRK_val ("plotoptions.code.printout");
	$plotadds = getRK_val ("plotoptions.code.calculate");
	if ($fun == "hist") {
		$normFun = "dnorm"; // draw normal density on the histogram
		$histcalcoptions = getRK_val ("histogram_opt.code.calculate"); // options that goes into hist() function
		$histplotoptions = getRK_val ("histogram_opt.code.printout"); // options that goes into plot.histogram()
		$histplotoptions .= $plotoptions; // generic plot options
	} elseif ($fun == "dist") {
		$normFun = "pnorm"; // draw normal cdf on the ecdf plot
		$plotoptions .= getRK_val ("dist_stepfun.code.printout"); // plot.ecdf() and plot.stepfun() options
	}

	$yLim = ""; // initialise the ylim option

?>
# generate the entire data:
<?
	doGenerateData (); // generate the random samples

?>
# get the sample averages:
avg <- colMeans(data);
<?
	if ($scalenorm) {
?>
# normalise the variables:
avg <- (avg - avg.exp)/sqrt(avg.var);
<?
	}
	if ($drawnorm) {
?>
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=<? echo ($nDist); ?>);
normY <- <? echo ($normFun); ?> (normX<? echo ($normMuSigma_tag); ?>);
<?
	}
	if ($fun == "hist") {
?>
dist.hist <- hist(avg, plot=FALSE<? echo ($histcalcoptions); ?>);
<?
		if ($drawnorm) {
?>
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
<?
			$yLim = ', ylim=ylim';
		}
	}
	if ($final) {
?>
rk.graph.on ()
try ({
<?
	}
	if (!empty ($plotpre)) printIndented ("\t", $plotpre);
	if ($fun == "hist") {
?>
	plot(dist.hist<? echo ($yLim); echo ($histplotoptions); ?>)
<?
	} elseif ($fun == "dist") {
?>
	plot(ecdf(avg)<? echo ($plotoptions); ?>)
<?
	}
	if ($drawnorm) {
?>
	lines (x=normX, y=normY, type="<? getRK ("normpointtype"); ?>"<? getRK ("normlinecol.code.printout"); ?>)
<?
	}
	if (!empty ($plotadds)) printIndented ("\t", $plotadds);
	if ($final) {
?>
})
rk.graph.off ()
<?
	}
}
?>