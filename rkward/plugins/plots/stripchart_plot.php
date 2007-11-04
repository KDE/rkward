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
	$x = getRK_val ("x");
	$g = getRK_val ("g");
	$method = '"' . getRK_val ("method") . '"';
	$jitter = getRK_val ("jitter");
	$offset = getRK_val ("offset");
	$vertical = getRK_val ("vertical");
	$plot_adds = getRK_val ("plotoptions.code.calculate"); //add grid and alike

	if ($final) { ?>
rk.header ("Stripchart", list ("Variable", rk.get.description (<? echo ($x); ?>), "Group", rk.get.description (<? echo ($g); ?>), "Method", <? echo ($method); ?>, "Jitter", <? echo ($jitter); ?>, "Plot drawn vertically", <? echo ($vertical); ?>, "Offset", <? echo ($offset); ?>))

rk.graph.on ()
<?	} ?>
try (stripchart (<? echo ($x); ?> ~ (<? echo ($g); ?>), vertical=<? echo ($vertical); ?>, method = <? echo ($method); ?>, jitter = <? echo ($jitter); ?>, offset = <? echo ($offset); ?><? getRK ("plotoptions.code.printout"); ?>))
<?	if (!empty ($plot_adds)) { ?>

<?		// print the grid() related code
		printIndented ("\t", $plot_adds);
	}
?>
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}
?>
