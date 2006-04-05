<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$xvarsstring = join (", ", split ("\n", getRK_val ("x")));
	$xvars = "list (" . $xvarsstring . ")";
	$xvarsstring = str_replace (array ("\\", "\""), array ("\\\\", "\\\""), $xvarsstring);
?>
rk.header ("Boxplot", list ("Variable(s)", "<?echo ($xvarsstring); ?>"))
rk.graph.on()
boxplot (<? echo ($xvars); ?>, notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation") ?><? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
