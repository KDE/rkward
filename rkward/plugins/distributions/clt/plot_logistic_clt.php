<?php
include ("plot_clt_common.php");

function doParameters () {
?>
loc <- <? echo(getRK_val ("loc")); ?>; scale <- <? echo(getRK_val ("scale")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- loc;
avg.var <- ((pi^2/3)*scale^2)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rlogis(n=<? echo ($nAvg*$nDist); ?>, location=loc, scale=scale), nrow=<? echo ($nAvg); ?>);
<?
}
?>
