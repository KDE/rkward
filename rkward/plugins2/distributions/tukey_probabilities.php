<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
result <- (ptukey (q = <? echo ($q); ?>, nmeans = <? getRK ("nmeans"); ?>, df = <? getRK ("df"); ?>, nranges = <? getRK ("nranges"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Studentized Range probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Sample size for range", "<? getRK ("nmeans"); ?>", "Degrees of freedom for s", "<? getRK ("df"); ?>", "Number of groups whose maximum range is considered", "<? getRK ("nranges"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (result, titles="Studentized Range probabilities")
<?
}
?>
