<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pt (q = <? getRK ("val"); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("r probability", list ("Variable value", "<? getRK ("val"); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>t probability:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
