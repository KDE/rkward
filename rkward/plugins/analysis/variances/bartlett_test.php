<?
function preprocess () {
}

function calculate () {

?>
result <- bartlett.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>)
<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)

rk.header ("Bartlett Test of Homogeneity of Variances",
	parameters=list ("Sample: ", paste (names[1], "grouped by:", names[2])))

rk.results (list (
	'Variables'=names,
	'Bartlett s K-squared'=result$statistic,
	'df'=result$parameter,
	'p-value'=result$p.value))
<?
}
?>
