<?
function preprocess () {
	global $dodensity_plot;
	$dodensity_plot = (getRK_val ("plot_type") == "density_plot");

	if (!$dodensity_plot) { ?>
require(hdrcde)
<?	}
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
	global $dodensity_plot;
	$adjust = getRK_val ("adjust");
	$x = getRK_val ("x");
	$resolution = getRK_val ("n");
	$narm = getRK_val ("narm");
	$kern = getRK_val ("kern");
	if ($kern == "gaussian") {
		$bw = getRK_val ("bw");
	}
	$dorug = getRK_val ("rug");

	if ($final) {
		if ($dodensity_plot) { ?>
rk.header ("Density Plot", list ("Variable", rk.get.description (<? echo ($x); ?>)<? if (!empty ($bw)) { ?>, "Band Width", "<? echo ($bw); ?>"<? } ?>, "Adjust", <? echo ($adjust) ?>, "Remove Missing Values", <? echo ($narm) ?>, "Length", length (<? echo ($x); ?>), "Resolution", <? echo ($resolution); ?>, "Smoothing Kernel", "<? echo ($kern); ?>"))
<?		} else { ?>
rk.header ("Highest density regions", list ("Variable", rk.get.description (<? echo ($x); ?>)))
<?		} ?>

rk.graph.on ()
<?	}
		?>
try ({
<?	if ($dodensity_plot) { ?>
	plot (density(<? echo ($x); if (!empty ($bw)) echo (", bw=\"" . $bw . "\""); ?>, adjust = <? echo ($adjust); ?>, kern = "<? echo ($kern); ?>", n = <? echo ($resolution); ?>, <? echo ($narm); ?>)<? getRK ("plotoptions.code.printout"); ?>)
<?	} else { ?>
	hdr.den(<? echo ($x); ?><? getRK ("plotoptions.code.printout"); ?>)
<?	}
	if ($dorug) { ?>
	rug(<? echo ($x); ?>, <? getRK ("rug_ticksize"); ?>, <? getRK ("rug_lwd"); ?>, <? getRK ("rug_side"); ?><? getRK ("rug_col.code.printout"); ?>)
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}
?>
