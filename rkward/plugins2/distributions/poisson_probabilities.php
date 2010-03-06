<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
result <- (ppois (q = <? echo ($q); ?>, lambda = <? getRK ("lambda"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Poisson probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Lambda", "<? getRK ("lambda"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Poisson probabilities")
<?
}
?>
