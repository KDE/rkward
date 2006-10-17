<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qlnorm (p = <? echo ($p); ?>, meanlog = <? getRK ("meanlog"); ?>, sdlog = <? getRK ("sdlog"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Log Normal quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "meanlog", "<? getRK ("meanlog"); ?>", "sdlog", "<? getRK ("sdlog"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
cat ("<h3>Log Normal quantiles:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
