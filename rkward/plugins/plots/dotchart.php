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
	$names_mode = getRK_val ("names_mode");
	$tabulate= getRK_val ("tabulate")=="TRUE";
	$tabulate= getRK_val ("tabulate");
	if ($tabulate) {
		$tabulate_header = '"Tabulate", "Yes"';
	} else {
		$tabulate_header = '"Tabulate", "No"';
	}

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
rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (<? echo ($vars); ?>), <? echo ($tabulate_header); ?>))

rk.graph.on ()
<?	}
?>
try ({
<?	if ($names_mode == "rexp") {
		echo ("names(x) <- " . getRK_val ("names_exp") . "\n");
	} else if ($names_mode == "custom") {
		echo ("names(x) <- c (\"" . str_replace (";", "\", \"", trim (getRK_val ("names_custom"))) . "\")\n");
	}
	if (!empty ($plotpre)) printIndented ("\t", $plotpre); ?>
	dotchart(x<? echo ($options); ?>)
<?	if (!empty ($plotpost)) printIndented ("\t", $plotpost); ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
