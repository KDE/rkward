<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
?>rk.header ("Plot density <? getRK ("function"); ?>", list ("Number of Observations", "<? getRK ("n"); ?>", "Minimum", "<? getRK ("min"); ?>", "Maximum", "<? getRK ("max"); ?>", "Numerator degrees of freedom", "<? getRK ("df1"); ?>", "Denominator degrees of freedom", "<? getRK ("df2"); ?>", "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (<? getRK ("function"); ?> (seq(<? getRK ("min"); ?> ,<? getRK ("max"); ?>, length= <? getRK ("n"); ?>) , df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>)))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
