<?php
include ("plot_clt_common.php");

function doParameters () {
?>
scale <- <? echo(getRK_val ("scale")); ?>; shape <- <? echo(getRK_val ("shape")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- scale*gamma(1+1/shape);
avg.var <- (scale^2*gamma(1+2/shape) - avg.exp^2)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rweibull(n=<? echo ($nAvg*$nDist); ?>, shape=shape, scale=scale), nrow=<? echo ($nAvg); ?>);
<?
}
?>
