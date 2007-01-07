<?
        function preprocess () {
        }

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(nortest)

rk.temp.vars <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.vars)), check.names=FALSE)
i=0;
for (var in rk.temp.vars) {
	i = i+1
	rk.temp.results$'Variable Name'[i] <- rk.get.description (var, is.substitute=TRUE)
	<? if (getRK_val ("length")) { ?>
	rk.temp.results$'Length'[i] <- try (length (eval (var)))
	<? }
	if (getRK_val ("nacount")) { ?>
	rk.temp.results$'NAs'[i] <- try (length (which(is.na(eval (var)))))
	<? } ?>
	try ({
		rk.temp.test <- shapiro.test (eval (var))
		rk.temp.results$'Statistic'[i] <- paste (names (rk.temp.test$statistic), rk.temp.test$statistic, sep=" = ")
		rk.temp.results$'p-value'[i] <- rk.temp.test$p.value
	})
}
<?
        }
	function printout () {
?>
rk.header ("Shapiro-Wilk Normality Test")
rk.results (rk.temp.results)
<?
        }
	function cleanup () {

?>
rm (rk.temp.results)
rm (rk.temp.vars)
rm (rk.temp.test)
rm (var)
<?
        }
?>