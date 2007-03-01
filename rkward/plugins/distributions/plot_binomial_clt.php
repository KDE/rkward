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

	$plotoptions = getRK_val("plotoptions.code.printout");
	if ($fun == "hist") {
		$normFun = "dnorm";
		$histcalcoptions = getRK_val ("histogram_opt.code.calculate");
		$histplotoptions = getRK_val ("histogram_opt.code.printout");
		$histplotoptions .= $plotoptions;
	} elseif ($fun == "dist") {
		$ecdfoptions = "";
		$col_y0 = getRK_val ("col_y0.code.printout");
		$col_y1 = getRK_val ("col_y1.code.printout");
		if (($col_y0 != "") && ($col_y1 != "")) {
			$ecdfoptions .= ", col.01line=c({$col_y0},{$col_y1})";
		} elseif (($col_y0 != "") || ($col_y1 != "")) {
			$ecdfoptions .= ", col.01line={$col_y0}{$col_y1}";
		}

		$addRugtoplot = getRK_val ("addRugtoplot");
		if ($addRugtoplot) {
			$rugoptions  = ', ticksize=' . round(getRK_val ("rug_ticksize"),2);
			$rugoptions .= ', lwd=' . round(getRK_val ("rug_lwd"),2);
			$rugoptions .= ', side=' . getRK_val ("rug_side");
			$rugoptions .= getRK_val ("col_rug.code.printout");
		}

		$normFun = "pnorm";
		$plotoptions .= $ecdfoptions . getRK_val ("dist_stepfun.code.printout");
	}

	$yLim = "";
?>
rk.temp.cltdistrib <- list()
rk.temp.cltdistrib$data <- matrix(rbinom(n=<? echo ($nAvg*$nDist); ?>, size = <? echo ($size); ?>, prob=<? echo ($prob); ?>), nrow=<? echo ($nAvg); ?>);
rk.temp.cltdistrib$avg <- colMeans(rk.temp.cltdistrib$data);
rk.temp.cltdistrib$mean <- <? echo ($distExp); ?>;
rk.temp.cltdistrib$var <- <? echo ($distVar); ?>;
<? if ($scalenorm) { ?>
rk.temp.cltdistrib$avg <- (rk.temp.cltdistrib$avg - rk.temp.cltdistrib$mean)/sqrt(rk.temp.cltdistrib$var);
<? }
 if ($drawnorm) { ?>
rk.temp.cltdistrib$normX <- seq(from=min(rk.temp.cltdistrib$avg), to=max(rk.temp.cltdistrib$avg), length=<? echo ($nDist); ?>);
rk.temp.cltdistrib$normY <- <? echo ($normFun); ?> (rk.temp.cltdistrib$normX, mean = <? echo ($normMu); ?>, sd = <? echo ($normSigma); ?>);
<? }
	if ($final) { ?>
rk.graph.on ()
<? }
  if ($fun == "hist") {
?>
rk.temp.cltdistrib$hist <- hist(rk.temp.cltdistrib$avg, plot=FALSE<? echo ($histcalcoptions); ?>);
<? if ($drawnorm) { ?>
rk.temp.cltdistrib$ylim <- c(0,max(c(rk.temp.cltdistrib$hist$density, rk.temp.cltdistrib$normY)));
<? $yLim = ', ylim=rk.temp.cltdistrib$ylim'; } ?>
try( plot(rk.temp.cltdistrib$hist<? echo ($yLim); echo ($histplotoptions); ?>) );
<?  } elseif ($fun == "dist") {?>
try( plot(ecdf(rk.temp.cltdistrib$avg)<? echo ($plotoptions); ?>) );
<?	if ($addRugtoplot) { ?>	rug (rk.temp.cltdistrib$avg<? echo ($rugoptions); ?>)
<? } }
 if ($drawnorm) { ?>
  try (lines (x=rk.temp.cltdistrib$normX, y=rk.temp.cltdistrib$normY, type="<? getRK ("normpointtype"); ?>"<? getRK ("normlinecol.code.printout"); ?>));
<? }
	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
