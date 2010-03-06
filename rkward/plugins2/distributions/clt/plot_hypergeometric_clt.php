<?php
include ("plot_clt_common.php");

function doParameters () {
?>
m <- <? echo(getRK_val ("m")); ?>; n <- <? echo(getRK_val ("n")); ?>; k <- <? echo(getRK_val ("k")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- k*m/(m+n);
avg.var <- (k*m*n*(m+n-k)/((m+n)^2*(m+n-1)))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rhyper(nn=<? echo ($nAvg*$nDist); ?>, m=m, n=n, k=k), nrow=<? echo ($nAvg); ?>);
<?
}
?>
