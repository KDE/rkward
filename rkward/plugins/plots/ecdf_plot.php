<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$x = getRK_val ("x");
?>
rk.header ("ECDF", list ("Variable", rk.get.description (<? echo ($x); ?>)))
rk.graph.on ()
plot.ecdf (<? echo ($x); ?>, <? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
