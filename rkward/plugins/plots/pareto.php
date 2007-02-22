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
?>
rk.temp.x <- (<? echo ($vars); ?>)
if(is.factor(rk.temp.x)) {
	rk.temp.x <- summary(rk.temp.x)
} 

<?	if ($final) { ?>
rk.header ("Pareto chart")

rk.graph.on ()
<?	}
?>
try ({
<?	if ($final && $descriptives) { ?>
	rk.temp.descriptives <- <?
	} else echo ("\t"); ?>
pareto.chart(rk.temp.x<? getRK ("plotoptions.code.printout"); ?>)
<?	if ($final && $descriptives) { ?>
	rk.print(xtable(rk.temp.descriptives))
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
