<?php
include ("plot_clt_common.php");

function doParameters () {
?>
shape <- <? echo(getRK_val ("shape")); ?>; rate <- <? echo(getRK_val ("rate")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- shape/rate;
avg.var <- (shape/(rate^2))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rgamma(n=<? echo ($nAvg*$nDist); ?>, shape=shape, rate=rate), nrow=<? echo ($nAvg); ?>);
<?
}
?>
