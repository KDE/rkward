<?
function preprocess () {
}

function calculate () {
}

function printout () {
	$breaks = getRK_val ("breaks");
	$scale = getRK_val ("scale");
	$bw =  getRK_val ("bw");
	$adjust = getRK_val ("adjust");
	$narm =  getRK_val ("narm");
	$giveRkern =  getRK_val ("giveRkern");
	$x = getRK_val ("x");
	if ($breaks == "int") {
		$breaksopt = "breaks =seq (floor (min (" . $x . "))-0.5, ceiling (max (" . $x ."))+0.5)";
		$breaks = "Integers";
	} else if (!empty ($breaks)) {
		$breaksopt = "breaks=\"" . $breaks . "\"";
	}
?>
rk.header ("Histogram", list ("Frequency", "<? echo $scale; ?>", "Breaks algorithm", <? echo ("\"" . $breaks . "\""); ?>, "Variable", rk.get.description (<? echo ($x); ?>)))
rk.graph.on ()
hist (<? echo ($x); ?>, <? echo ($breaksopt); ?>, freq = <? echo $scale; ?><? getRK ("plotoptions.code.printout"); ?>)
<?	if (($scale=="FALSE") && getRK_val ("density")) { ?>
lines(density(<? echo ($x); ?>, bw="<? echo ($bw); ?>", adjust = <? echo ($adjust); ?>, <? echo ($giveRkern); ?>, <? echo ($narm); ?>, n = <? getRK ("n"); ?>), col= "<? getRK ("col_density"); ?>")
<?	} ?>
rk.graph.off ()
<?
}

function cleanup () {
}
?>
