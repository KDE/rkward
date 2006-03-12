<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qbinom (p = <? getRK ("p"); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>))
rk.temp.date = date()
<?
	}
	
	function printout () {

		//produce the output
?>
cat ("<h1>Binomial quantile</h1>")

	cat ("<table border = \"1\">")
		cat ("<TR><TD>Method:</TD><TD>Probabilities [0,1] = <? getRK ("p"); ?></TD><TD>Binomial trials =  <? getRK ("size"); ?></TD><TD>Probability of success =  <? getRK ("prob"); ?></TD><TD><? getRK ("tail"); ?></TD></TR>")
		cat ("<TR><TD>", rk.temp.date,"</TD></TR>")
			cat ("<TR><TD></TD></TR>")
		cat ("<TR><TD>Binomial quantile:</TD><TD> ", rk.temp," </TD></TR>")
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
