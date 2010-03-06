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
	$x = getRK_val ("xvarslot");
	$y = getRK_val ("yvarslot");
	if (!($y == "")) {
		$y = ", " . $y;
	}
	// get additional code (as of now grid) from the calculate section
	$plot_adds = getRK_val ("plotoptions.code.calculate");

	if ($final) {
?>
rk.header ("Generic Plot")
rk.graph.on ()
<?
	}
?>
try({
	plot(<? echo ($x . $y . getRK_val ("plotoptions.code.printout")); ?>);
<?	if (!empty ($plot_adds)) { ?>

<?		// print the grid() related code
		printIndented ("\t", $plot_adds);
	}
?>
})
<?
	if ($final) {
?>
rk.graph.off ()
<?
	}
}
?>
