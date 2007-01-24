<?
function preprocess () {
}

function calculate () {
}

function printout () {
	global $scale;
	global $bw;
	global $adjust;
	global $narm;
	global $giveRkern;
	global $x;
	global $breaksopt;

	preparedata ();
?>
rk.header ("Histogram", list ("Frequency", "<? echo $scale; ?>", "Breaks algorithm", <? echo ("\"" . $breaks . "\""); ?>, "Variable", rk.get.description (<? echo ($x); ?>)))
rk.graph.on ()
<?	makeplotcommand (); ?>
rk.graph.off ()
<?
}

function cleanup () {
}

function preview () {
	preparedata ();
	makeplotcommand ();
}



// internal helper functions below;
function preparedata () {
	global $scale;
	global $bw;
	global $adjust;
	global $narm;
	global $giveRkern;
	global $x;
	global $breaksopt;

	$breaks = getRK_val ("breaks");
	$scale = getRK_val ("scale");
	$bw =  getRK_val ("bw");
	$adjust = getRK_val ("adjust");
	$narm =  getRK_val ("narm");
	$giveRkern =  getRK_val ("giveRkern");
	$x = getRK_val ("x");
	if ($breaks == "int") {
		$breaksopt = "breaks =seq (floor (min (" . $x . ", na.rm=TRUE))-0.5, ceiling (max (" . $x .", na.rm=TRUE))+0.5)";
		$breaks = "Integers";
	} else if (!empty ($breaks)) {
		$breaksopt = "breaks=\"" . $breaks . "\"";
	}
}

function makeplotcommand () {
	global $scale;
	global $bw;
	global $adjust;
	global $narm;
	global $giveRkern;
	global $x;
	global $breaksopt;
?>
try ({
	hist (<? echo ($x); ?>, <? echo ($breaksopt); ?>, freq = <? echo $scale; ?><? getRK ("plotoptions.code.printout"); ?>)
<?	if (($scale=="FALSE") && getRK_val ("density")) { ?>
	lines(density(<? echo ($x); ?>, bw="<? echo ($bw); ?>", adjust = <? echo ($adjust); ?>, <? echo ($giveRkern); ?>, <? echo ($narm); ?>, n = <? getRK ("n"); ?>), col= "<? getRK ("col_density"); ?>")
<?	} ?>
})
<?
}

?>
