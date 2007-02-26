<?
function preprocess () {
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function cleanup () {
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);	// only this one actually needed...
	cleanup ();
}

// internal helper functions
function doPrintout ($final) {
	$densityscaled = getRK_val ("densityscaled");
	$bw =  getRK_val ("bw");
	$adjust = getRK_val ("adjust");
	$narm =  getRK_val ("narm");
	$giveRkern =  getRK_val ("giveRkern");
	$x = getRK_val ("x");

	if ($final) { ?>
rk.header ("Histogram", list ("Variable", rk.get.description (<? echo ($x); ?>)<? getRK ("histogram_opt.code.preprocess"); ?>))

rk.graph.on ()
<?	}
?>
try ({
	hist (<? echo ($x); getRK ("histogram_opt.code.calculate"); getRK ("histogram_opt.code.printout"); getRK ("plotoptions.code.printout"); ?>)
<?	if (($densityscaled) && getRK_val ("density")) { ?>
	lines(density(<? echo ($x); ?>, bw="<? echo ($bw); ?>", adjust = <? echo ($adjust); ?>, <? echo ($giveRkern); ?>, <? echo ($narm); ?>, n = <? getRK ("n"); ?>)<? getRK ("col_density.code.printout"); ?>)
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}

?>
