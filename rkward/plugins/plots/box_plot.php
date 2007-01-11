<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$xvarsstring = join (", ", split ("\n", getRK_val ("x")));
?>
rk.header ("Boxplot", list ("Variable(s)", rk.get.description (<?echo ($xvarsstring); ?>, paste.sep=", ")))
rk.graph.on()
try (boxplot (list (<? echo ($xvarsstring); ?>), notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation") ?><? getRK ("plotoptions.code.printout"); ?>))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
