<?
function preprocess () { ?>
require(qcc)
<?	if (getRK_val ("descriptives")=="TRUE") { ?>
require(xtable)
<?	}
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
	$descriptives = getRK_val ("descriptives")=="TRUE";
	$tabulate= getRK_val ("tabulate")=="TRUE";

if($tabulate) {?>
x <- table (<? echo ($vars); ?>, exclude=NULL)
<?      } else { ?>
x <- <? echo ($vars); ?>

if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}
<?      } ?>

<?	if ($final) { ?>
rk.header ("Pareto chart")

rk.graph.on ()
<?	}
?>
try ({
	descriptives <- pareto.chart(x<? getRK ("plotoptions.code.printout"); ?>)
<?	if ($final && $descriptives) { ?>
	rk.results(xtable(descriptives))
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
