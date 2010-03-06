<?
function preprocess () { ?>
require(vcd)
<? }

function calculate () {
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function doPrintout ($final) {
	$vars = getRK_val ("x");
	$shade = getRK_val ("shade");
	$sievetype = getRK_val ("sievetype");
{ ?>
x <- <? echo ($vars); ?>
<? } ?>

<?	if ($final) { ?>
rk.header ("Extended Sieve Plot", parameters=list ("Variable", rk.get.description (<? echo ($vars); ?>), "shade", <? echo ($shade); ?>))

rk.graph.on ()
<?	}
?>
try ({
<?	if (!empty ($plotpre)) printIndented ("\t", $plotpre);
	
?>
	sieve(x, shade = <? echo ($shade); ?>, sievetype = "<? echo ($sievetype); ?>" <? getRK ("plotoptions.code.printout"); ?>)
<?	if (!empty ($plotpost)) printIndented ("\t", $plotpost); ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
