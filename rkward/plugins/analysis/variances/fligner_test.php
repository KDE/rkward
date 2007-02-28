<?
function preprocess () {
}

function calculate () {

?>
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- bartlett.test (eval (rk.temp.x), eval (rk.temp.y))

<?
}

function printout () {
?>
rk.header ("Fligner-Killeen Test of Homogeneity of Variances",
	parameters=list ("Sample", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "Group", rk.get.description (rk.temp.y, is.substitute=TRUE))))

rk.results (list (
	'Variables'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'Fligner-Killeen:med X^2 test statistic'=rk.temp$statistic,
	'df'=rk.temp$parameter,
	'p-value'=rk.temp$p.value
	))
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
