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
		rk.temp.t <- outlier (eval (var), opposite = <? getRK ("opposite"); ?>, logical = <? getRK ("logical"); ?>)
		rk.temp.results$'Outlier'[i] <- rk.temp.t
	})
	<? if (getRK_val ("mean")) { ?>
	try (rk.temp.results$'Mean'[i] <- mean (eval (var)))
	<? } ?>
	<? if (getRK_val ("sd")) { ?>
	try (rk.temp.results$'Standard Deviation'[i] <- sd (eval (var)))
	<? } ?>
	<? if (getRK_val ("median")) { ?>
	try (rk.temp.results$'Median'[i] <- median (eval (var)))
	<? } ?>
	<? if (getRK_val ("min")) { ?>
	try (rk.temp.results$'Minimum'[i] <- min (eval (var)))
	<? } ?>
	<? if (getRK_val ("max")) { ?>
	try (rk.temp.results$'Maximum'[i] <- max (eval (var)))
	<? } ?>
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
rk.header ("Outlier",
	parameters=list ("Opposite", "<? getRK ("opposite"); ?>", "Logical", "<? getRK ("logical"); ?>"))
rk.results (rk.temp.results)
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
