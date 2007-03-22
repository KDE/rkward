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
	$fun = getRK_val ("function");
	$nAvg = getRK_val ("nAvg"); // number of observations to calculate the averages
	$nDist = getRK_val ("nDist"); // number of sample to construct the distribution

	$scalenorm = getRK_val ("scalenorm"); // if variables should to normalised..
	$drawnorm = getRK_val ("drawnorm");

	// We need df>2 for variance to exist. Taken care of in xml file: using "is_var_gt2" in <logic>
?>
# parameters:
df <- <? echo(getRK_val ("df")."\n"); ?>
ncp <- <? echo(getRK_val ("ncp")."\n"); ?>
<?
	if ($scalenorm || $drawnorm) {
		if ($ncp == "0") { // mean and variance of the distribution of sample averages
?>
# mean and variances of the distribution of sample averages:
avg.exp <- 0;
avg.var <- df/((df-2)*<? echo($nAvg); ?>)
<?
		} else {
?>
# mean and variances of the distribution of sample averages:
tmp.var <- gamma((df-1)/2)/gamma(df/2)
avg.exp <- ncp*sqrt(df/2)*tmp.var
avg.var <- (df*(1+ncp^2)/(df-2) - ncp^2*df*tmp.var^2/2)/<? echo($nAvg); ?>
<?
		}
	}
	// Mean and Std.deviantion of Normal distribution:
	if ($scalenorm) $normMuSigma_tag = "";
	else	$normMuSigma_tag = ", mean = avg.exp, sd = sqrt(avg.var)";

	$plotoptions = getRK_val("plotoptions.code.printout");
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
data <- matrix(rt(n=<? echo ($nAvg*$nDist); ?>, df=df, ncp=ncp), nrow=<? echo ($nAvg); ?>);
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
	if ($final) {
?>
	})
rk.graph.off ()
<?
	}
}
?>
