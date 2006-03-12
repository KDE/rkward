<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qnorm (p = <? getRK ("p"); ?>, mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>, <? getRK ("tail"); ?>))
rk.temp.date = date()
<?
	}
	
	function printout () {

		//produce the output
?>
cat ("<h1>Normal quantile</h1>")

	cat ("<table border = \"1\">")
		cat ("<TR><TD>Method:</TD><TD>n = <? getRK ("p"); ?></TD><TD>mu =  <? getRK ("mean"); ?></TD><TD>sigma = <?getRK ("sd"); ?></TD><TD><? getRK ("tail"); ?></TD></TR>")
		cat ("<TR><TD>", rk.temp.date,"</TD></TR>")
			cat ("<TR><TD></TD></TR>")
		cat ("<TR><TD>Normal quantile:</TD><TD> ", rk.temp," </TD></TR>")
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
