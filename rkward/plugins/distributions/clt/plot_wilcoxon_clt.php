<?php
include ("plot_clt_common.php");

function doParameters () {
?>
m <- <? echo(getRK_val ("nm")); ?>; n <- <? echo(getRK_val ("nn")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- m*n/2;
avg.var <- (m*n*(m+n+1)/12)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rwilcox(nn=<? echo ($nAvg*$nDist); ?>, m=m, n=n), nrow=<? echo ($nAvg); ?>);
<?
}
?>
