<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (plogis (q = <? echo ($q); ?>, location = <? getRK ("location"); ?>, scale = <? getRK ("scale"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Logistic probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Location", "<? getRK ("location"); ?>", "Scale", "<? getRK ("scale"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Logistic probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
