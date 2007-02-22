<?
function preprocess () {
?> require(qcc) 
require(xtable)
<?
}
	
function calculate () {
}

function printout () {
	doPrintout (true);
}
	
function cleanup () { ?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
	
function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
	cleanup ();
}
	
function doPrintout ($final) {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>
rk.temp.x <- (<? echo ($vars); ?>)
if(is.factor(rk.temp.x)) {
	rk.temp.x <- summary(rk.temp.x)
} else rk.temp.x

<? 
	if ($final) { ?>
rk.header ("Pareto chart")

rk.graph.on ()
<?	}
?>
try ({ 
<?
if ($final) { ?>
rk.print(<?}?>pareto.chart(rk.temp.x, main="") <? if ($final) { ?>)<?}
?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
