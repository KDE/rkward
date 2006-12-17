<?
	function preprocess () {
	}
	
	function calculate () {
?>
	rk.min <- min (<? getRK("x"); ?>);
	rk.max <- max (<? getRK("x"); ?>);
	rk.length <- length (<? getRK ("x"); ?>);
	doth_pnrom <- <? getRK ("th_pnorm"); ?>;
<?
	}
	
	function printout () {
	$x = getRK_val ("x");
	$col = getRK_val ("col");
?>
rk.header ("ECDF", list ("Variable", rk.get.description (<? echo ($x); ?>), "Minimum", rk.min, "Maximum", rk.max, "Length", rk.length))
rk.graph.on ()
plot.ecdf (<? echo ($x); ?>, <? getRK ("dopoints"); ?>, <? getRK ("verticals"); ?> <? getRK ("plotoptions.code.printout"); ?>)
if (doth_pnrom) cat (paste (curve(pnorm, from= rk.min, to= rk.max, add=TRUE, col="<? echo ($col); ?>")))
rug (<? getRK ("x"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
?>	rm (rk.min)
	rm (rk.max)
	rm (rk.length)
	rm (doth_pnrom)
<?
	}
?>
