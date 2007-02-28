<?
	function preprocess () {
	}

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(outliers)

rk.temp.objects <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.objects)), check.names=FALSE)
i=0;
for (var in rk.temp.objects) {
	i = i+1
	rk.temp.results$'Variable Name'[i] <- rk.get.description (var, is.substitute=TRUE)
	try ({
		rk.temp.t <- PP.test (eval (var), lshort = <? getRK ("lshort"); ?>)
		rk.temp.results$'Dickey-Fuller'[i] <- rk.temp.t$statistic
		rk.temp.results$'Truncation lag parameter'[i] <- rk.temp.t$parameter
		rk.temp.results$'p-value'[i] <- rk.temp.t$p.value
	})
}
<?
        }

function printout () {
?>
rk.header ("Phillips-Perron Test for Unit Roots",
	parameters=list ("Truncation lag parameter short ('TRUE') or long ('FALSE')", "<? getRK ("lshort"); ?>"))
rk.results (rk.temp.results)
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
