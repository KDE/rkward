<?
function preprocess () {
}

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
	$tabulate= getRK_val ("tabulate")=="TRUE";

	$options = getRK_val ("plotoptions.code.printout");

	$plotpre = getRK_val ("plotoptions.code.preprocess");
	$plotpost = getRK_val ("plotoptions.code.calculate");

if($tabulate) {?>
x <- table (<? echo ($vars); ?>, exclude=NULL)
<?      } else { ?>
x <- <? echo ($vars); ?>

if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}
<?      } ?>

<?	if ($final) { ?>
rk.header ("Dot chart", list ("Variable", rk.get.description (<? echo ($vars); ?>)))

rk.graph.on ()
<?	}
?>
try ({
<?	if (!empty ($plotpre)) printIndented ("\t", $plotpre); ?>
	dotchart(x<? echo ($options); ?>)
<?	if (!empty ($plotpost)) printIndented ("\t", $plotpost); ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
