<?
function preprocess () {
}

function calculate () {

?>
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- var.test (eval (rk.temp.x), eval (rk.temp.y), alternative = "<? getRK ("alternative"); ?>")

<?
}

function printout () {
?>
rk.header ("Mood Two-Sample Test of Scale",
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "and", rk.get.description (rk.temp.y, is.substitute=TRUE)), "Alternative Hypothesis", "<? getRK ("alternative"); ?>"))

rk.results (list (
	'Variables'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'Z'=rk.temp$statistic,
	'p-value'=rk.temp$p.value,
	'Alternative Hypothesis'=rk.describe.alternative(rk.temp)
	))
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
