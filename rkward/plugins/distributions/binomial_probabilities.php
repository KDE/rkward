<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pbinom (q = <? echo ($q); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $q;
	//produce the output
?>
rk.header ("Binomial tail probability", list ("Variable value", "<? echo ($q); ?>", "Binomial trials", "<? getRK ("size"); ?>", "Probability of success", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>"));
rk.results (rk.temp, titles="Binomial tail probability")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
