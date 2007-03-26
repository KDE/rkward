<?
function preprocess () {
}

function calculate () {
	global $vars;
	$vars = str_replace ("\n", ", ", trim (getRK_val ("x")));
?>
result <- bartlett.test (list (<? echo ($vars); ?>))
<?
}

function printout () {
	global $vars;
?>
names <- rk.get.description (<? echo ($vars); ?>)

rk.header (result$method)

rk.results (list (
	'Variables'=names,
	'Bartlett s K-squared'=result$statistic,
	'df'=result$parameter,
	'p-value'=result$p.value))
<?
}
?>
