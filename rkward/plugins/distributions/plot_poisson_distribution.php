<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
?>rk.header ("Plot density <? getRK ("function"); ?>", list ("Minimum", "<? getRK ("min"); ?>", "Maximum", "<? getRK ("max"); ?>", "Mean", "<? getRK ("mean"); ?>", "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
plot (<? getRK ("function"); ?> (<? getRK ("min"); ?> : <? getRK ("max"); ?>, lambda = <? getRK ("mean"); ?>))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
