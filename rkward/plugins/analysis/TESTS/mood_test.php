<?
function preprocess () {
}

function calculate () {
?>
result <- mood.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>, alternative = "<? getRK ("alternative"); ?>")
<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)

rk.header (result$method,
	parameters=list ("Alternative Hypothesis", rk.describe.alternative (result)))

rk.results (list (
	'Variables'=names,
	'Z'=result$statistic,
	'p-value'=result$p.value))
<?
}
?>
