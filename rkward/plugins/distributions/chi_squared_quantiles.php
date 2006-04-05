<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qchisq (p = <? getRK ("p"); ?>, df = <? getRK ("df"); ?>, ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Chi-squared quantile", list ("Probabilities [0,1]", "<? getRK ("p"); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Chi-squared quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
