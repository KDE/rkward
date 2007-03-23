<?php
include ("plot_clt_common.php");

function doParameters () {
?>
mean <- <? echo(getRK_val ("mean")); ?>; sd <- <? echo(getRK_val ("sd")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- mean;
avg.var <- (sd^2)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rnorm(n=<? echo ($nAvg*$nDist); ?>, mean=mean, sd=sd), nrow=<? echo ($nAvg); ?>);
<?
}
?>
