<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
result <- (qunif (p = <? echo ($p); ?>, min = <? getRK ("min"); ?>, max = <? getRK ("max"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Uniform quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Lower limits of the distribution", "<? getRK ("min"); ?>", "Upper limits of the distribution", "<? getRK ("max"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Uniform quantiles")
<?
}
?>
