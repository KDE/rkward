<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qt (p = <? echo ($p); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("t quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
rk.results (rk.temp, titles="t quantiles")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
