<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pt (q = <? getRK ("val"); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
rk.temp.date = date()
<?
	}
	
	function printout () {

		//produce the output
?>
cat ("<h1>t probaility</h1>")

	cat ("<table border = \"1\">")
		cat ("<TR><TD>Method:</TD><TD>Variable value : <? getRK ("val"); ?></TD><TD>Degree of Freedom :  <? getRK ("df"); ?></TD><TD><? getRK ("tail"); ?></TD></TR>")
		cat ("<TR><TD>", rk.temp.date,"</TD></TR>")
			cat ("<TR><TD></TD></TR>")
		cat ("<TR><TD>t probaility:</TD><TD> ", rk.temp," </TD></TR>")
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
