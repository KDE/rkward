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
	$vars = getRK_val ("x");
	$descriptives = getRK_val ("descriptives")=="TRUE";
	$tabulate= getRK_val ("tabulate")=="TRUE";

if($tabulate) {?>
rk.temp.x <- table (<? echo ($vars); ?>, exclude=NULL)
<?      } else { ?>
rk.temp.x <- <? echo ($vars); ?>

if (!is.numeric (rk.temp.x)) {
       rk.print ("Data may not be numeric, but proceeding as requested.\nDid you
forget to check the tabulate option?")
}
<?      } ?>

<?	if ($final) { ?>
rk.header ("Pareto chart")

rk.graph.on ()
<?	}
?>
try ({
	rk.temp.descriptives <- pareto.chart(rk.temp.x<? getRK ("plotoptions.code.printout"); ?>)
<?	if ($final && $descriptives) { ?>
	rk.print(xtable(rk.temp.descriptives))
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
