<?php
include ("plot_clt_common.php");

function doParameters () {
?>
llim <- <? echo(getRK_val ("llim")); ?>; ulim <- <? getRK ("ulim"); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- (llim+ulim)/2;
avg.var <- ((ulim-llim)^2/12)/<? echo($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(runif(n=<? echo ($nAvg*$nDist); ?>, min=llim, max=ulim), nrow=<? echo ($nAvg); ?>);
<?
}
?>
