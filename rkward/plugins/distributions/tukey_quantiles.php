<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qtukey (p = <? echo ($p); ?>, nmeans = <? getRK ("nmeans"); ?>, df = <? getRK ("df"); ?>, nranges = <? getRK ("nranges"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Studentized Range quantiles", list ("Vector of probabilities", "<? echo ($p); ?>", "Sample size for range", "<? getRK ("nmeans"); ?>", "Degrees of freedom for s", "<? getRK ("df"); ?>", "Number of groups whose maximum range is considered", "<? getRK ("nranges"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
cat ("<h3>Studentized Range quantiles:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
