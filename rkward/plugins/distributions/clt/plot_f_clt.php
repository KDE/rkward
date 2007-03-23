<?php
include ("plot_clt_common.php");

function doParameters () {
?>
df1 <- <? echo(getRK_val ("df1")); ?>; df2 <- <? echo(getRK_val ("df2")); ?>; ncp <- <? echo(getRK_val ("ncp")); ?>;
<?
}

function doExpVar () {
	global $nAvg;

	if (getRK_val ("ncp") == 0) {
?>
avg.exp <- df2*df1/(df1*(df2-2));
avg.var <- (2*df2^2*(df1+df2-2)/(df1*(df2-2)^2*(df2-4)))/<? echo($nAvg); ?>;
<?
	} else {
?>
avg.exp <- df2*(df1+ncp)/(df1*(df2-2));
avg.var <- (2*df2^2*((df1+ncp)^2  + (df1+2*ncp)*(df2-2)) / (df1^2*(df2-2)^2*(df2-4)))/<? echo($nAvg); ?>;
<?
	}
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rf(n=<? echo ($nAvg*$nDist); ?>, df1=df1, df2=df2, ncp=ncp), nrow=<? echo ($nAvg); ?>);
<?
}
?>
