<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
result <- (pweibull (q = <? echo ($q); ?>, shape = <? getRK ("shape"); ?>, scale = <? getRK ("scale"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Weibull probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Shape", "<? getRK ("shape"); ?>", "Scale", "<? getRK ("scale"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Weibull probabilities")
<?
}
?>
