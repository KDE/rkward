<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
	$breaks = getRK_val ("breaks");
	$scale = getRK_val ("scale");
	$x = getRK_val ("x");
	if ($breaks == "int") {
		$breaksopt = "breaks=seq (as.integer (min (" . $x . "))-0.5, as.integer (max (" . $x ."))+0.5)";
		$breaks = "Integers";
	} else if (!empty ($breaks)) {
		$breaksopt = "breaks=\"" . $breaks . "\"";
	}
?>
rk.header ("Histogram", list ("Frequency", "<? echo $scale; ?>", "Breaks algorithm", <? echo ("\"" . $breaks . "\""); ?>, "Variable", rk.get.description (<? echo ($x); ?>)))
rk.graph.on ()
hist (<? echo ($x); ?>, <? echo ($breaksopt); ?>, freq = <? echo $scale; ?><? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
