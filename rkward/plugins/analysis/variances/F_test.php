<?
function preprocess () {
}

function calculate () {

?>
result <- var.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>, alternative = "<? getRK ("alternative"); ?>", ratio = <? getRK ("ratio"); ?><?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)

rk.header (result$method,
	parameters=list ("Confidence Level", "<? getRK ("conflevel"); ?>", "Alternative Hypothesis", rk.describe.alternative(result)))

rk.results (list (
	'Variables'=names,
	'F'=result$statistic["F"],
	'Numerator DF'=result$parameter["num df"],
	'Denominator DF'=result$parameter["denom df"],
	'p-value'=result$p.value,
	'Lower CI'=result$conf.int[1],
	'Upper CI'=result$conf.int[2],
	'ratio of variances'=result$estimate))
<?
}
?>
