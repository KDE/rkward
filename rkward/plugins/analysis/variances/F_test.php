<?
function preprocess () {
}

function calculate () {

?>
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- var.test (eval (rk.temp.x), eval (rk.temp.y), alternative = "<? getRK ("alternative"); ?>", ratio = <? getRK ("ratio"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
rk.header ("F test to compare two variances",
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "against", rk.get.description (rk.temp.y, is.substitute=TRUE)),"Confidence Level", "<? getRK ("conflevel"); ?>", "Ratio", "<? getRK ("ratio"); ?>", "Alternative Hypothesis", "<? getRK ("alternative"); ?>"))

rk.results (list (
	'Variables'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'F'=rk.temp$statistic["F"],
	'Numerator DF'=rk.temp$parameter["num df"],
	'Denominator DF'=rk.temp$parameter["denom df"],
	'p-value'=rk.temp$p.value,
	'Alternative Hypothesis'=rk.describe.alternative(rk.temp),
	'Lower CI'=rk.temp$conf.int[1],
	'Upper CI'=rk.temp$conf.int[2],
	'ratio of variances'=rk.temp$estimate
	))
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
