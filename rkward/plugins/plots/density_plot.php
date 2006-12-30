<?
	function preprocess () {
	}
	
	function calculate () {
?>	
	length.temp <- length (<? getRK ("x"); ?>);
	kern <- c("<? getRK ("kern"); ?>");
	bw <- c("<? getRK ("bw"); ?>");
	dohdrcde_plot <- <? getRK ("dohdrcde_plot"); ?>;
	dodensity_plot <- <? getRK ("dodensity_plot"); ?>;
	dorug_density <- <? getRK ("rug_density"); ?>;
	dorug_hdrcde <- <? getRK ("rug_hdrcde"); ?>;
<?
	}
	
	function printout () {
	$giveRkern = getRK_val ("giveRkern");
	$adjust = getRK_val ("adjust");
	$bw = getRK_val ("bw");
	$x = getRK_val ("x");
	$narm = getRK_val ("narm");
	$kern = getRK_val ("kern");
?>

if (dodensity_plot) rk.header ("Density Plot", list ("Variable", rk.get.description (<? echo ($x); ?>), "Band Width", bw, "Estimate Density", <? echo ($giveRkern) ?>, "Adjust", <? echo ($adjust) ?>, "Remove Missing Values", <? echo ($narm) ?>, "Length", length.temp, "n", <? getRK ("n"); ?>, "Smoothing Kernel", kern))

if (dohdrcde_plot) rk.header ("Highest density regions", list ("Variable", rk.get.description (<? echo ($x); ?>)))

rk.graph.on ()

if ((dohdrcde_plot) && (dodensity_plot)) par(mfrow=c(1,2))

if (dodensity_plot) plot (density(<? echo ($x); ?>, bw = "<? echo ($bw); ?>", adjust = <? echo ($adjust); ?>, <? echo ($giveRkern); ?>, kern = c("<? echo ($kern) ?>"), n = <? getRK ("n"); ?>, <? echo ($narm); ?> <? getRK ("plotoptions.code.printout"); ?>))
if (dorug_density) rug(<? echo ($x); ?>, <? getRK ("ticksize"); ?>, <? getRK ("lwd"); ?>, <? getRK ("side"); ?>, col ="<? getRK ("col_rug"); ?>")
if (dohdrcde_plot) require(hdrcde)
if (dohdrcde_plot) hdr.den(<? echo ($x); ?> <? getRK ("plotoptions.code.printout"); ?>)
if (dorug_hdrcde) rug(<? echo ($x); ?>, <? getRK ("ticksize"); ?>, <? getRK ("lwd"); ?>, <? getRK ("side"); ?>, col ="<? getRK ("col_rug"); ?>")
rk.graph.off ()
<?
	}
	
	function cleanup () {
?>	rm (length.temp)
	rm (kern)
	rm (bw)
	rm (dohdrcde_plot)
	rm (dodensity_plot)
	rm (dorug_hdrcde)
	rm (dorug_density)
<?
	}
?>
