<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qchisq (p = <? echo ($p); ?>, df = <? getRK ("df"); ?>, ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Chi-squared quantile", list ("Probabilities [0,1]", "<? echo ($p); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Chi-squared quantile:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
