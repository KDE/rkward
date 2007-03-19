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
	$nm = getRK_val ("nm");
	$nn = getRK_val ("nn");
	$nAvg = getRK_val ("nAvg"); // number of observations to calculate the averages
	$nDist = getRK_val ("nDist"); // number of sample to construct the distribution

	$scalenorm = getRK_val ("scalenorm"); // if variables should to normalised..
	$drawnorm = getRK_val ("drawnorm");

	$distExp = $nm*$nn/2; // mean of the distribution of sample averages
	$distVar = $nm*$nn*($nm+$nn+1)/(12*$nAvg); // variance of the distribution of sample averages

	if ($scalenorm) {
		$normMu = 0; // mean for normal
		$normSigma = 1; // std dev for normal
	} else {
		$normMu = $distExp;
		$normSigma = sqrt($distVar);
	}

	$plotoptions = getRK_val("plotoptions.code.printout");
	if ($fun == "hist") {
		$normFun = "dnorm"; // draw normal density on the histogram
		$histcalcoptions = getRK_val ("histogram_opt.code.calculate"); // options that goes into hist() function
		$histplotoptions = getRK_val ("histogram_opt.code.printout"); // options that goes into plot.histogram()
		$histplotoptions .= $plotoptions; // generic plot options
	} elseif ($fun == "dist") {
		$ecdfoptions = "";
		$col_y0 = getRK_val ("col_y0.code.printout");
		$col_y1 = getRK_val ("col_y1.code.printout");
		if (($col_y0 != "") && ($col_y1 != "")) {
			$ecdfoptions .= ", col.01line=c({$col_y0},{$col_y1})";
		} elseif (($col_y0 != "") || ($col_y1 != "")) {
			$ecdfoptions .= ", col.01line={$col_y0}{$col_y1}";
		} // col.01line option to plot.ecdf()

		$normFun = "pnorm"; // draw normal cdf on the ecdf plot
		$plotoptions .= $ecdfoptions . getRK_val ("dist_stepfun.code.printout"); // plot.ecdf() and plot.stepfun() options
	}

	$yLim = ""; // initialise the ylim option
?>
# generate the entire data:
data <- matrix(rwilcox(nn=<? echo ($nAvg*$nDist); ?>, m = <? echo ($nm); ?>, n = <? echo ($nn); ?>), nrow=<? echo ($nAvg); ?>);
# get the sample averages:
avg <- colMeans(data);
<?
	if ($scalenorm) {
?>
# mean for the sample averages:
dist.mean <- <? echo ($distExp); ?>;
# variance for the sample averages:
dist.var <- <? echo ($distVar); ?>;
# normalise the variables:
avg <- (avg - dist.mean)/sqrt(dist.var);
<?
	}
	if ($drawnorm) {
?>
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=<? echo ($nDist); ?>);
normY <- <? echo ($normFun); ?> (normX, mean = <? echo ($normMu); ?>, sd = <? echo ($normSigma); ?>);
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
