<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qt (p = <? echo ($p); ?>, df = <? getRK ("df"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("t quantile", list ("Probabilities [0,1]", "<? echo ($p); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>t quantile:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
