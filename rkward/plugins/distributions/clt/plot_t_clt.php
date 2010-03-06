<?php
include ("plot_clt_common.php");

function doParameters () {
?>
df <- <? echo(getRK_val ("df")); ?>; ncp <- <? getRK ("ncp"); ?>;
<?
}

function doExpVar () {
	global $nAvg;

	if (getRK_val ("ncp") == 0) {
?>
avg.exp <- 0;
avg.var <- df/((df-2)*<? echo($nAvg); ?>);
<?
	} else {
?>
tmp.var <- gamma((df-1)/2)/gamma(df/2);
avg.exp <- ncp*sqrt(df/2)*tmp.var;
avg.var <- (df*(1+ncp^2)/(df-2) - ncp^2*df*tmp.var^2/2)/<? echo($nAvg); ?>;
<?
	}
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rt(n=<? echo ($nAvg*$nDist); ?>, df=df, ncp=ncp), nrow=<? echo ($nAvg); ?>);
<?
}
?>
