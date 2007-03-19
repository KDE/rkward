<?
function preprocess () {
}

function calculate () {
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout ($final) {
	$xvarsstring = join (", ", split ("\n", getRK_val ("x")));

	if ($final) {
?>
rk.header ("Boxplot", list ("Variable(s)", rk.get.description (<? echo ($xvarsstring); ?>, paste.sep=", ")))
rk.graph.on()
<?	} ?>
try (boxplot (list (<? echo ($xvarsstring); ?>), notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation") ?><? getRK ("plotoptions.code.printout"); ?>))
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}
?>
