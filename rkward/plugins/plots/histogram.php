<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
?>
rk.header ("Histogram", list ("Frequency", "<? getRK ("scale"); ?>"))
cat ("<h1>Histogram</h1>")
rk.graph.on ()
plot (hist (<? getRK ("x"); ?>, freq = <? getRK ("scale"); ?>)<? getRK ("plotoptions.code.printout"); ?>)
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
