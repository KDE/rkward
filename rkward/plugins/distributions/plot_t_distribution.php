<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp.date = date()
<?
	}
	
	function printout () {
?>cat ("<h1>Plot density <? getRK ("function"); ?></h1>")
rk.graph.on ()
plot (<? getRK ("function"); ?> (seq(<? getRK ("min"); ?> ,<? getRK ("max"); ?>, length= <? getRK ("n"); ?>) , mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>))
rk.graph.off ()
cat ("<table border = \"1\">")
		cat ("<TR><TD>Created</TD><TD> ", rk.temp.date," </TD></TR>")
		cat ("<TR><TD>Number of Observations</TD><TD><? getRK ("n"); ?></TD></TR>")
		cat ("<TR><TD>Minimum</TD><TD><? getRK ("min"); ?></TD></TR>")
		cat ("<TR><TD>Maximum</TD><TD><? getRK ("max"); ?></TD></TR>")
		cat ("<TR><TD>Mean</TD><TD><? getRK ("mean"); ?></TD></TR>")
		cat ("<TR><TD>Standard Deviation</TD><TD><? getRK ("sd"); ?></TD></TR>")
		cat ("<TR><TD>Function</TD><TD><? getRK ("function"); ?></TD></TR>")
	cat ("</table>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp.date)
rm (rk.temp.seq)
<?
	}
?>
