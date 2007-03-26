<?
function preprocess () {
}

function calculate () {

?>
result <- var.test (<? getRK ("x"); ?>, <? getRK ("x"); ?>, alternative = "<? getRK ("alternative"); ?>", ratio = <? getRK ("ratio"); ?><?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)

rk.header ("F test to compare two variances",
	parameters=list ("Comparing", paste (names[1], "against", names[2]),"Confidence Level", "<? getRK ("conflevel"); ?>", "Ratio", "<? getRK ("ratio"); ?>", "Alternative Hypothesis", "<? getRK ("alternative"); ?>"))

rk.results (list (
	'Variables'=names,
	'F'=result$statistic["F"],
	'Numerator DF'=result$parameter["num df"],
	'Denominator DF'=result$parameter["denom df"],
	'p-value'=result$p.value,
	'Alternative Hypothesis'=rk.describe.alternative(result),
	'Lower CI'=result$conf.int[1],
	'Upper CI'=result$conf.int[2],
	'ratio of variances'=result$estimate))
<?
}
?>
