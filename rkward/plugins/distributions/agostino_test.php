<?
        function preprocess () {
        }

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(moments)

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
		rk.temp.test <- agostino.test (eval (var), alternative = c("<? getRK ("alternative"); ?>"))
		rk.temp.results$'Statistic'[i] <- paste (names (rk.temp.test$statistic), rk.temp.test$statistic, sep=" = ")
		rk.temp.results$'p-value'[i] <- rk.temp.test$p.value
	})

}
<?
        }
	function printout () {
?>
rk.header ("D'Agostino test for skewness in normally distributed data")
rk.results (rk.temp.results)
<?
        }
	function cleanup () {

?>
rm (rk.temp.results)
rm (rk.temp.test)
rm (rk.temp.vars)
rm (var)
<?
        }
?>