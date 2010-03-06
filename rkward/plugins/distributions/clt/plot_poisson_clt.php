<?php
include ("plot_clt_common.php");

function doParameters () {
?>
mean <- <? echo(getRK_val ("mean")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- mean;
avg.var <- (mean)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rpois(n=<? echo ($nAvg*$nDist); ?>, lambda=mean), nrow=<? echo ($nAvg); ?>);
<?
}
?>
