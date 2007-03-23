<?php
include ("plot_clt_common.php");

function doParameters () {
?>
df <- <? echo(getRK_val ("df")); ?>; ncp <- <? echo(getRK_val ("ncp")); ?>;
<?
}

function doExpVar () {
	global $nAvg;

	if (getRK_val ("ncp") == 0) {
?>
avg.exp <- df;
avg.var <- (2*df)/<? echo ($nAvg); ?>;
<?
	} else {
?>
avg.exp <- df + ncp;
avg.var <- (2*df + 4*ncp)/<? echo ($nAvg); ?>;
<?
	}
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rchisq(n=<? echo ($nAvg*$nDist); ?>, df=df, ncp=ncp), nrow=<? echo ($nAvg); ?>);
<?
}
?>
