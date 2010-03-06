<?php
include ("plot_clt_common.php");

function doParameters () {
?>
size <- <? echo(getRK_val ("size")); ?>; prob <- <? echo(getRK_val ("prob")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- size*prob;
avg.var <- (size*prob*(1-prob))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rbinom(n=<? echo ($nAvg*$nDist); ?>, size=size, prob=prob), nrow=<? echo ($nAvg); ?>);
<?
}
?>
