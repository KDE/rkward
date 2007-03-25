<?php
include ("plot_clt_common.php");

function doParameters () {
	global $paramTag;
 	if ( getRK_val ("param") == "pprob") {
		$paramTag = ", prob=prob";
?>
size <- <? echo(getRK_val ("size_trial")); ?>; prob <- <? echo(getRK_val ("prob")); ?>;
<?
	} else {
		$paramTag = ", mu=mu";
?>
size <- <? echo(getRK_val ("size_disp")); ?>; mu <- <? echo(getRK_val ("mu")); ?>; prob <- size/(size+mu);
<?
	}
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- size*(1-prob)/prob;
avg.var <- (size*(1-prob)/prob^2)/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $paramTag;
	global $nAvg;
	global $nDist;
?>
data <- matrix(rnbinom(n=<? echo ($nAvg*$nDist); ?>, size=size<? echo ($paramTag); ?>), nrow=<? echo ($nAvg); ?>);
<?
}
?>
