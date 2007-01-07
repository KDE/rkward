<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pbinom (q = <? echo ($q); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Binomial tail probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Binomial trials", "<? getRK ("size"); ?>", "Probability of success", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (rk.temp, titles="Binomial tail probabilities")
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
