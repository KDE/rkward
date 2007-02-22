<?
        function preprocess () {
        }

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(moments)

rk.temp.objects <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.objects)), check.names=FALSE)
i=0;
for (var in rk.temp.objects) {
	i = i+1
	rk.temp.results$'Variable Name'[i] <- rk.get.description (var, is.substitute=TRUE)
	<? 
	if (getRK_val ("skewness")) { ?>
	try (rk.temp.results$'Skewness'[i] <- skewness (eval (var), <? getRK ("narm_skewness"); ?>))
	<? }
	if (getRK_val ("kurtosis")) { ?>
	try (rk.temp.results$'Kurtosis'[i] <- kurtosis (eval (var), <? getRK ("narm_kurtosis"); ?>))
	<? }
	if (getRK_val ("geary")) { ?>
	try (rk.temp.results$'Geary kurtosis'[i] <- geary (eval (var), <? getRK ("narm_geary_kurtosis"); ?>))
	<? }
	if (getRK_val ("length")) { ?>
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
rk.header ("Skewness and Kurtosis")
rk.results (rk.temp.results)
<?
        }
	function cleanup () {

?>
	rm (rk.temp.results)
	rm (rk.temp.objects)
	rm (var)
<?
        }
?>