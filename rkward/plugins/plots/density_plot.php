<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$giveRkern = getRK_val ("giveRkern");
	$adjust = getRK_val ("adjust");
	$bw = getRK_val ("bw");
	$x = getRK_val ("x");
	$narm = getRK_val ("narm");
	$kern = "c(\"" . getRK_val ("kern") . "\")";
	$dodensity_plot = getRK_val ("dodensity_plot");
	$dohdrcde_plot = getRK_val ("dohdrcde_plot");
	$dorug_density = getRK_val ("rug_density");
	$dorug_hdrcde = getRK_val ("rug_hdrcde");

	if ($dodensity_plot) { ?>
rk.header ("Density Plot", list ("Variable", rk.get.description (<? echo ($x); ?>), "Band Width", c("<? getRK ("bw"); ?>"), "Estimate Density", <? echo ($giveRkern) ?>, "Adjust", <? echo ($adjust) ?>, "Remove Missing Values", <? echo ($narm) ?>, "Length", length (<? echo ($x); ?>), "n", <? getRK ("n"); ?>, "Smoothing Kernel", <? echo ($kern); ?>))
<?	}
	if ($dohdrcde_plot) { ?>
rk.header ("Highest density regions", list ("Variable", rk.get.description (<? echo ($x); ?>)))
<?	} ?>	

rk.graph.on ()

<?	if ($dohdrcde_plot && $dodensity_plot) { ?>
par(mfrow=c(1,2))
<?	} ?>

<?	if ($dodensity_plot) { ?>
plot (density(<? echo ($x); ?>, bw = "<? echo ($bw); ?>", adjust = <? echo ($adjust); ?>, <? echo ($giveRkern); ?>, kern = <? echo ($kern); ?>, n = <? getRK ("n"); ?>, <? echo ($narm); ?><? getRK ("plotoptions.code.printout"); ?>))
<?	} ?>

<?	if ($dorug_density) { ?>
rug(<? echo ($x); ?>, <? getRK ("ticksize"); ?>, <? getRK ("lwd"); ?>, <? getRK ("side"); ?>, col ="<? getRK ("col_rug"); ?>")
<?	}

	if ($dohdrcde_plot) { ?>
require(hdrcde)
hdr.den(<? echo ($x); ?><? getRK ("plotoptions.code.printout"); ?>)
<?	}
	if ($dorug_hdrcde) { ?>
rug(<? echo ($x); ?>, <? getRK ("ticksize"); ?>, <? getRK ("lwd"); ?>, <? getRK ("side"); ?>, col ="<? getRK ("col_rug"); ?>")
	<? } ?>

rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
