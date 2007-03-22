<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
result <- (qnbinom (p = <? echo ($p); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Negative Binomial quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Size", "<? getRK ("size"); ?>", "Probability of success in each trial", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Negative Binomial quantiles")
<?
}
?>
