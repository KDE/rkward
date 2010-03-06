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
	if ($method == "\"jitter\"") {
		$opts .= ", jitter = " . getRK_val ("jitter");
		$params .= ", \"Jitter\" = " . getRK_val ("jitter");
	} else if ($method == "\"stack\"") {
		$opts .= ", offset = " . getRK_val ("offset");
		$params .= ", \"Offset\" = " . getRK_val ("offset");
	}
	$orientation = getRK_val ("orientation");
	if ($orientation == "Vertical") $opts .= ", vertical = TRUE";
	$plot_adds = getRK_val ("plotoptions.code.calculate"); //add grid and alike

	if ($final) { ?>
rk.header ("Stripchart", list ("Variable"=rk.get.description (<? echo ($x); ?>), "Group"=rk.get.description (<? echo ($g); ?>), "Method"=<? echo ($method); echo ($params); ?>, "Orientation"="<? echo ($orientation); ?>"))

rk.graph.on ()
<?	} ?>
try (stripchart (<? echo ($x); ?> ~ (<? echo ($g); ?>), method = <? echo ($method); echo ($opts); getRK ("plotoptions.code.printout"); ?>))
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
