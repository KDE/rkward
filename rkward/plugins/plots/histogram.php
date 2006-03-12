<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp.date = date()
<?
	}
	
	function printout () {
?>cat ("<h1>Histogram</h1>")
rk.graph.on ()
plot (hist (<? getRK ("x"); ?>, freq = <? getRK ("scale"); ?>))
rk.graph.off ()
cat ("<table border = \"1\">")
		cat ("<TR><TD>Created</TD><TD> ", rk.temp.date," </TD></TR>")
		cat ("<TR><TD>Frequency</TD><TD><? getRK ("scale"); ?></TD></TR>")
	cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp.date)
<?
	}
?>
