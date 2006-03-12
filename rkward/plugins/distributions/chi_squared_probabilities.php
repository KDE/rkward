<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pchisq (q = <? getRK ("val"); ?>, df = <? getRK ("df"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
rk.temp.date = date()
<?
	}
	
	function printout () {

		//produce the output
?>
cat ("<h1>Chi-squared probaility</h1>")

	cat ("<table border = \"1\">")
		cat ("<TR><TD>Method:</TD><TD>Variable value = <? getRK ("val"); ?></TD><TD>Degree of Freedom =  <? getRK ("df"); ?></TD><TD>ncp = <? getRK ("ncp"); ?></TD><TD><? getRK ("tail"); ?></TD></TR>")
		cat ("<TR><TD>", rk.temp.date,"</TD></TR>")
			cat ("<TR><TD></TD></TR>")
		cat ("<TR><TD>Chi-squared probaility:</TD><TD> ", rk.temp," </TD></TR>")
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
