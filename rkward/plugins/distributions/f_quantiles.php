<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qf (p = <? getRK ("p"); ?>, df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>, <? getRK ("tail"); ?>))
rk.temp.date = date()
<?
	}
	
	function printout () {

		//produce the output
?>
cat ("<h1>F quantile</h1>")

	cat ("<table border = \"1\">")
		cat ("<TR><TD>Method:</TD><TD>Probabilities [0,1] : <? getRK ("p"); ?></TD><TD>Numerator degree of freedom :  <? getRK ("df1"); ?></TD><TD>Denominator degree of freedom :  <? getRK ("df2"); ?></TD><TD><? getRK ("tail"); ?></TD></TR>")
		cat ("<TR><TD>", rk.temp.date,"</TD></TR>")
			cat ("<TR><TD></TD></TR>")
		cat ("<TR><TD>F quantile:</TD><TD> ", rk.temp," </TD></TR>")
	cat ("</table>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
rm (rk.temp.date)
<?
	}
?>
