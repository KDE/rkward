<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp <- (pchisq (q = <? getRK ("q"); ?>, df = <? getRK ("df"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Chi-squared probability", list ("Variable value", "<? getRK ("q"); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Chi-squared probaility:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
