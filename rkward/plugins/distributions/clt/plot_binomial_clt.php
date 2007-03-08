<?php
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {
	doPrintout (true);
  }

  function cleanup () {
?>
rm(rk.temp.cltdistrib)
<?
	}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
	cleanup ();
}

function doPrintout ($final) {
	$fun = getRK_val ("function");
	$size = getRK_val ("size");
	$prob = getRK_val ("prob");
	$nAvg = getRK_val ("nAvg"); // number of observations to calculate the averages
	$nDist = getRK_val ("nDist"); // number of sample to construct the distribution

	$scalenorm = getRK_val ("scalenorm"); // if variables should to normalised..
	$drawnorm = getRK_val ("drawnorm");

	$distExp = $size*$prob; // mean of the distribution of sample averages
	$distVar = $size*$prob*(1-$prob)/$nAvg; // variance of the distribution of sample averages

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
rk.temp.cltdistrib <- list()
# generate the entire data:
rk.temp.cltdistrib$data <- matrix(rbinom(n=<? echo ($nAvg*$nDist); ?>, size = <? echo ($size); ?>, prob=<? echo ($prob); ?>), nrow=<? echo ($nAvg); ?>);
# get the sample averages:
rk.temp.cltdistrib$avg <- colMeans(rk.temp.cltdistrib$data);
<?
	if ($scalenorm) {
?>
# mean for the sample averages:
rk.temp.cltdistrib$mean <- <? echo ($distExp); ?>;
# variance for the sample averages:
rk.temp.cltdistrib$var <- <? echo ($distVar); ?>;
# normalise the variables:
rk.temp.cltdistrib$avg <- (rk.temp.cltdistrib$avg - rk.temp.cltdistrib$mean)/sqrt(rk.temp.cltdistrib$var);
<?
	}
	if ($drawnorm) {
?>
# generate random normal samples:
rk.temp.cltdistrib$normX <- seq(from=min(rk.temp.cltdistrib$avg), to=max(rk.temp.cltdistrib$avg), length=<? echo ($nDist); ?>);
rk.temp.cltdistrib$normY <- <? echo ($normFun); ?> (rk.temp.cltdistrib$normX, mean = <? echo ($normMu); ?>, sd = <? echo ($normSigma); ?>);
<?
	}
  if ($fun == "hist") {
?>
rk.temp.cltdistrib$hist <- hist(rk.temp.cltdistrib$avg, plot=FALSE<? echo ($histcalcoptions); ?>);
<?
	if ($drawnorm) {
?>
# calculate the ylims appropriately:
rk.temp.cltdistrib$ylim <- c(0,max(c(rk.temp.cltdistrib$hist$density, rk.temp.cltdistrib$normY)));
<?
		$yLim = ', ylim=rk.temp.cltdistrib$ylim';
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
	plot(rk.temp.cltdistrib$hist<? echo ($yLim); echo ($histplotoptions); ?>)
<?
	} elseif ($fun == "dist") {
?>
	plot(ecdf(rk.temp.cltdistrib$avg)<? echo ($plotoptions); ?>)
<?
	}
	if ($drawnorm) {
?>
	lines (x=rk.temp.cltdistrib$normX, y=rk.temp.cltdistrib$normY, type="<? getRK ("normpointtype"); ?>"<? getRK ("normlinecol.code.printout"); ?>)
<?
	}
	if ($final) {
?>
rk.graph.off ()
<?
	}
}
?>
