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
		rk.temp.t <- grubbs.test (eval (var), type = <? getRK ("type"); ?>, opposite = <? getRK ("opposite"); ?>, two.sided = <? getRK ("two_sided"); ?>)
		rk.temp.results$'G'[i] <- rk.temp.t$statistic["G"]
		rk.temp.results$'U'[i] <- rk.temp.t$statistic["U"]
		rk.temp.results$'p-value'[i] <- rk.temp.t$p.value
		rk.temp.results$'Alternative Hypothesis'[i] <- rk.temp.t$"alternative"
	})
	<? if (getRK_val ("length")) { ?>
	try (rk.temp.results$'Length'[i] <- length (eval (var)))
	<? }
	if (getRK_val ("nacount")) { ?>
	try (rk.temp.results$'NAs'[i] <- length (which(is.na(eval (var)))))
	<? } ?>
}
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
