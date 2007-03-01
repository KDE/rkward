<?
	function preprocess () {
	}

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(outliers)

rk.temp.objects <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.objects)), check.names=FALSE)
local({
	i=0;
	for (var in rk.temp.objects) {
		i = i+1
		rk.temp.results$'Variable Name'[i] <<- rk.get.description (var, is.substitute=TRUE)
		try ({
			rk.temp.t <- chisq.out.test (eval (var), opposite = <? getRK ("opposite"); ?>, variance = var (eval 	(var)))
			rk.temp.variance <- var (eval (var))
			rk.temp.results$'X-squared'[i] <<- rk.temp.t$statistic
			rk.temp.results$'p-value'[i] <<- rk.temp.t$p.value
			rk.temp.results$'Alternative Hypothesis'[i] <<- rk.describe.alternative(rk.temp.t)
			rk.temp.results$'Variance'[i] <<- rk.temp.variance
		})
		<? if (getRK_val ("mean")) { ?>
		try (rk.temp.results$'Mean'[i] <<- mean (eval (var)))
		<? } ?>
		<? if (getRK_val ("sd")) { ?>
		try (rk.temp.results$'Standard Deviation'[i] <<- sd (eval (var)))
		<? } ?>
		<? if (getRK_val ("median")) { ?>
		try (rk.temp.results$'Median'[i] <<- median (eval (var)))
		<? } ?>
		<? if (getRK_val ("min")) { ?>
		try (rk.temp.results$'Minimum'[i] <<- min (eval (var)))
		<? } ?>
		<? if (getRK_val ("max")) { ?>
		try (rk.temp.results$'Maximum'[i] <<- max (eval (var)))
		<? } ?>
		<? if (getRK_val ("length")) { ?>
		try (rk.temp.results$'Length'[i] <<- length (eval (var)))
		<? }
		if (getRK_val ("nacount")) { ?>
		try (rk.temp.results$'NAs'[i] <<- length (which(is.na(eval (var)))))
		<? } ?>
	}
})
<?
        }

function printout () {
?>
rk.header ("Chi-squared test for outlier",
	parameters=list ("Opposite", "<? getRK ("opposite"); ?>"))
rk.results (rk.temp.results)
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
