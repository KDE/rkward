<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (punif (q = <? echo ($q); ?>, min = <? getRK ("min"); ?>, max = <? getRK ("max"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Uniform probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Lower limits of the distribution", "<? getRK ("min"); ?>", "Upper limits of the distribution", "<? getRK ("max"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Uniform probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
