<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pt (q = <? echo ($q); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("t probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
rk.results (rk.temp, titles="t probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
