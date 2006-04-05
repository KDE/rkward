<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qnorm (p = <? getRK ("p"); ?>, mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Normal quantile", list ("Probabilities [0,1]", "<? getRK ("p"); ?>", "mu", "<? getRK ("mean"); ?>", "sigma", "<? getRK ("sd"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Normal quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
