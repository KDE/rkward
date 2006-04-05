<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp = (qbinom (p = <? getRK ("p"); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>))
<?
	}
	
	function printout () {

		//produce the output
?>
rk.header ("Binomial quantile", list ("Probabilities [0,1]", "<? getRK ("p"); ?>", "Binomial trials", "<? getRK ("size"); ?>", "Probability of success", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Binomial quantile:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
