<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
?>rk.header ("Plot density <? getRK ("function"); ?>", list ("Number of Observations", "<? getRK ("n"); ?>", "Minimum", "<? getRK ("min"); ?>", "Maximum", "<? getRK ("max"); ?>", "Mean", "<? getRK ("mean"); ?>", "Standard Deviation", "<? getRK ("sd"); ?>", "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (<? getRK ("function"); ?> (seq(<? getRK ("min"); ?> ,<? getRK ("max"); ?>, length= <? getRK ("n"); ?>) , mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>)))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
