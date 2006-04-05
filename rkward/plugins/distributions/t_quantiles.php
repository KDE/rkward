<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qt (p = <? getRK ("p"); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("t quantile", list ("Probabilities [0,1]", "<? getRK ("p"); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>t quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
