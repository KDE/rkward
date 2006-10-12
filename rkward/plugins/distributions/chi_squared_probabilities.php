<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pchisq (q = <? echo ($q); ?>, df = <? getRK ("df"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Chi-squared probability", list ("Variable value", "<? echo ($q); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Chi-squared probaility:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
