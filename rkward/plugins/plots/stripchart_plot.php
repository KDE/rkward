<?
	function preprocess () {
	}
	
	function calculate () {
?>	length.temp <- length (<? getRK ("x"); ?>);
	method.temp <- c("<? getRK ("method"); ?>");
<?
	}
	
	function printout () {
	$x = getRK_val ("x");
	$g = getRK_val ("g");
	$method = getRK_val ("method");
	$jitter = getRK_val ("jitter");
	$offset = getRK_val ("offset");
?>
rk.header ("Stripchart", list ("Variable", rk.get.description (<? echo ($x); ?>), "Length", length.temp, "Method", method.temp, "Jitter", <? echo ($jitter); ?>, "Plot drawn vertically", <? getRK ("vertical"); ?>, "Offset", <? getRK ("offset"); ?>))
rk.graph.on ()
stripchart (<? echo ($x); ?> ~ (<? echo ($g); ?>), vertical= <? getRK ("vertical"); ?>, method = "<? getRK ("method"); ?>", jitter = <? echo ($jitter); ?>, offset = <? echo ($offset); ?> <? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
?>	rm (length.temp)
	rm (method.temp)
<?
	}
?>
