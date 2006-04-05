<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (pbinom (q = <? getRK ("q"); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Binomial tail probability", list ("Variable value", "<? getRK ("q"); ?>", "Binomial trials", "<? getRK ("size"); ?>", "Probability of success", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>t probability:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
