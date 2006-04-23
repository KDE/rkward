<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pt (q = <? getRK ("q"); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("t probability", list ("Variable value", "<? getRK ("q"); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>t probability:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
