<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qf (p = <? echo ($p); ?>, df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("F quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Numerator degrees of freedom", "<? getRK ("df1"); ?>", "Denominator degrees of freedom", "<? getRK ("df2"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>F quantiles:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
