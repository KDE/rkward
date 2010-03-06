<?php
include ("plot_clt_common.php");

function doParameters () {
?>
shape1 <- <? echo(getRK_val ("a")); ?>; shape2 <- <? echo(getRK_val ("b")); ?>;
<?
}

function doExpVar () {
	global $nAvg;
?>
avg.exp <- shape1/(shape1 + shape2);
avg.var <- (shape1*shape2/((shape1+shape2)^2*(shape1+shape2+1)))/<? echo ($nAvg); ?>;
<?
}

function doGenerateData () {
	global $nAvg;
	global $nDist;
?>
data <- matrix(rbeta(n=<? echo ($nAvg*$nDist); ?>, shape1=shape1, shape2=shape2), nrow=<? echo ($nAvg); ?>);
<?
}
?>
