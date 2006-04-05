<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qf (p = <? getRK ("p"); ?>, df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("F quantile", list ("Probabilities [0,1]", "<? getRK ("p"); ?>", "Numerator degrees of freedom", "<? getRK ("df1"); ?>", "Denominator degrees of freedom", "<? getRK ("df2"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>F quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
