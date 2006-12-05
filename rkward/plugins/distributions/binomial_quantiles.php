<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qbinom (p = <? echo ($p); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}
	
function printout () {
	//produce the output
	global $p;
?>
rk.header ("Binomial quantile", list ("Vector of quantiles probabilities", "<? echo ($p); ?>", "Binomial trials", "<? getRK ("size"); ?>", "Probability of success", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
cat ("<h3>Binomial quantiles:  ", rk.temp, "</h3>")<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
