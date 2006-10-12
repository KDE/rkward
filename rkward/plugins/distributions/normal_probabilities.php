<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp = (pnorm (q = <? echo ($q); ?>, mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Normal probabilities", list ("Variable value(s)", "<? echo ($q); ?>", "mu", "<? getRK ("mean"); ?>", "sigma", "<? getRK ("sd"); ?>", "Tail", "<? getRK ("tail"); ?>"));
cat ("<h3>Normal quantile:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
