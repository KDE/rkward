<?php
include ("plot_clt_common.php");

function doParameters () {
?>
prob <- <? echo(getRK_val ("prob")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- (1-prob)/prob;
avg.var <- ((1-prob)/(prob^2))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rgeom(n=<? echo ($nAvg*$nDist); ?>, prob=prob), nrow=<? echo ($nAvg); ?>);
<?
}
?>
