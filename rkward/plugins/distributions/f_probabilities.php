<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp <- (pf (q = <? getRK ("val"); ?>, df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("F probability", list ("Variable value", "<? getRK ("val"); ?>", "Numerator degrees of freedom", "<? getRK ("df1"); ?>", "Denominator degrees of freedom", "<? getRK ("df2"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>F probability:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
