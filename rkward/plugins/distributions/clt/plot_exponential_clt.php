<?php
include ("plot_clt_common.php");

function doParameters () {
?>
rate <- <? echo(getRK_val ("rate")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- 1/rate;
avg.var <- (1/(rate^2))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rexp(n=<? echo ($nAvg*$nDist); ?>, rate=rate), nrow=<? echo ($nAvg); ?>);
<?
}
?>
