<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$x = getRK_val ("x");
?>
rk.header ("Boxplot", list ("Variable", rk.get.description (<? echo ($x); ?>)))
rk.graph.on()
boxplot (<? echo ($x); ?>, notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation") ?><? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
