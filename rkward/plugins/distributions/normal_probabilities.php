<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pnorm (q = <? getRK ("val"); ?>, mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Normal probabilities", list ("Variable value(s)", "<? getRK ("val"); ?>", "mu", "<? getRK ("mean"); ?>", "sigma", "<? getRK ("sd"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Normal quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
