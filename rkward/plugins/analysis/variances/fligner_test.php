<?
function preprocess () {
}

function calculate () {
?>
result <- fligner.test (<? getRK ("x"); ?>, <? getRK ("x"); ?>)
<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)

rk.header ("Fligner-Killeen Test of Homogeneity of Variances",
	parameters=list ("Sample: ", paste (names[1], "grouped by:", names[2])))

rk.results (list (
	'Variables'=names,
	'Fligner-Killeen:med X^2 test statistic'=result$statistic,
	'df'=result$parameter,
	'p-value'=result$p.value))
<?
}

?>
