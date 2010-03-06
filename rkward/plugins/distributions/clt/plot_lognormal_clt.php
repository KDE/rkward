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
avg.exp <- exp(mean+sd^2/2);
avg.var <- (exp(2*mean+sd^2)*(exp(sd^2)-1))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rlnorm(n=<? echo ($nAvg*$nDist); ?>, meanlog=mean, sdlog=sd), nrow=<? echo ($nAvg); ?>);
<?
}
?>
