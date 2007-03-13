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
	for (sub in rk.temp.objects) {
		i = i+1
		rk.temp.results$'Variable Name'[i] <<- rk.get.description (sub, is.substitute=TRUE)
		var <- na.omit (eval (sub))
		try ({
			rk.temp.t <- grubbs.test (var, type = <? getRK ("type"); ?>, opposite = <? getRK ("opposite"); ?>, two.sided = <? getRK ("two_sided"); ?>)
			rk.temp.results$'G'[i] <<- rk.temp.t$statistic["G"]
			rk.temp.results$'U'[i] <<- rk.temp.t$statistic["U"]
			rk.temp.results$'p-value'[i] <<- rk.temp.t$p.value
			rk.temp.results$'Alternative Hypothesis'[i] <<- rk.describe.alternative (rk.temp.t)
		})
		<? if (getRK_val ("mean")) { ?>
		try (rk.temp.results$'Mean'[i] <<- mean (var))
		<? } ?>
		<? if (getRK_val ("sd")) { ?>
		try (rk.temp.results$'Standard Deviation'[i] <<- sd (var))
		<? } ?>
		<? if (getRK_val ("median")) { ?>
		try (rk.temp.results$'Median'[i] <<- median (var))
		<? } ?>
		<? if (getRK_val ("min")) { ?>
		try (rk.temp.results$'Minimum'[i] <<- min (var))
		<? } ?>
		<? if (getRK_val ("max")) { ?>
		try (rk.temp.results$'Maximum'[i] <<- max (var))
		<? } ?>
		<? if (getRK_val ("length")) { ?>
		try (rk.temp.results$'Length'[i] <<- length (eval (sub)))
		<? }
		if (getRK_val ("nacount")) { ?>
		try (rk.temp.results$'NAs'[i] <<- length (which(is.na(eval (sub)))))
		<? } ?>
	}
})
<?
        }

function printout () {
?>
rk.header ("Grubbs tests for one or two outliers in data sample",
	parameters=list ("Type", "<? getRK ("type"); ?>", "Opposite", "<? getRK ("opposite"); ?>", "two-sided", "<? getRK ("two_sided"); ?>"))
rk.results (rk.temp.results)
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
