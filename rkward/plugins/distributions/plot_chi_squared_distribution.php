<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
?>rk.header ("Plot density <? getRK ("function"); ?>", list ("Number of Observations", "<? getRK ("n"); ?>", "Minimum", "<? getRK ("min"); ?>", "Maximum", "<? getRK ("max"); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (<? getRK ("function"); ?> (seq(<? getRK ("min"); ?> ,<? getRK ("max"); ?>, length= <? getRK ("n"); ?>) , df = <? getRK ("df"); ?>), xlab=expression(chi^2)))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
