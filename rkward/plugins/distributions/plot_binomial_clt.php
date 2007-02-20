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
	$nAvg = getRK_val ("nAvg");
	$nDist = getRK_val ("nDist");

	$scalenorm = getRK_val ("scalenorm");
	$drawnorm = getRK_val ("drawnorm");

	$distExp = $size*$prob;
	$distVar = $size*$prob*(1-$prob)/$nAvg;

	if ($scalenorm) {
	 $normMu = 0;
	 $normSigma = 1;
	} else {
	 $normMu = $distExp;
	 $normSigma = sqrt($distVar);
	}

  if ($fun == "hist") {
    $histoptions = ", breaks=";

    $histbreaks = getRK_val ("histbreaksFunction");
    if ($histbreaks == "cells") $histoptions .= getRK_val ("histbreaks_ncells");
    else $histoptions .= "\"" . $histbreaks . "\"";

    $histlty = getRK_val ("histlinetype");
    $histoptions .= ", lty=" . "\"" . $histlty . "\"";
    if ($histlty != "blank") {
      $density = getRK_val ("density");
      $histoptions .= ", density=" . $density;

      if ($density > 0) $histoptions .= ", angle=" . getRK_val ("angle");

      $histbordercol = getRK_val ("histbordercol");
      if (($histbordercol != "FALSE") & ($histbordercol != "TRUE")) $histbordercol = "\"" . $histbordercol . "\"";
      $histfillcol = getRK_val ("histfillcol");
      if ($histfillcol != "NULL") $histfillcol = "\"" . $histfillcol . "\"";

      $histoptions .= ", border=" . $histbordercol . ", col=" . $histfillcol;
    }
    $histoptions .= getRK_val("plotoptions.code.printout");
  }
?>
rk.temp.cltdistrib <- list()
rk.temp.cltdistrib$data <- matrix(rbinom(n=<? echo ($nAvg*$nDist); ?>, size = <? echo ($size); ?>, prob=<? echo ($prob); ?>), nrow=<? echo ($nAvg); ?>);
rk.temp.cltdistrib$avg <- colMeans(rk.temp.cltdistrib$data);
rk.temp.cltdistrib$mean <- c(<? echo ($distExp); ?>)
rk.temp.cltdistrib$var <- c(<? echo ($distVar); ?>)
if (<? echo ($scalenorm); ?>) rk.temp.cltdistrib$avg <- (rk.temp.cltdistrib$avg - rk.temp.cltdistrib$mean)/sqrt(rk.temp.cltdistrib$var)
rk.temp.cltdistrib$normX <- seq(from=min(rk.temp.cltdistrib$avg), to=max(rk.temp.cltdistrib$avg), length=<? echo ($nDist); ?>)
rk.temp.cltdistrib$normY <- dnorm (rk.temp.cltdistrib$normX, mean = <? echo ($normMu); ?>, sd = <? echo ($normSigma); ?>)
<?
	if ($final) { ?>
rk.graph.on ()
<? }
  if ($fun == "hist") {
?>
rk.temp.cltdistrib$hist <- hist(rk.temp.cltdistrib$avg, plot=FALSE)
rk.temp.cltdistrib$ylim <- c(0,max(c(rk.temp.cltdistrib$hist$density, rk.temp.cltdistrib$normY)))
try( plot(rk.temp.cltdistrib$hist, freq=FALSE, ylim=rk.temp.cltdistrib$ylim<? echo ($histoptions); ?>) )
<?
  }
?>
if (<? echo ($drawnorm); ?>) {
  try (lines (x=rk.temp.cltdistrib$normX, y=rk.temp.cltdistrib$normY, type="<? getRK ("normpointtype"); ?>", col="<? getRK ("normlinecol"); ?>"))
}
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
