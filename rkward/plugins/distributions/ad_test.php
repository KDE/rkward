<?
function preprocess () {
}

function calculate () {
$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(nortest)

rk.temp.options <- list (dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)

rk.temp.vars <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.vars)))

i=0;
for (rk.temp.var in rk.temp.vars) {
	i = i+1
	rk.temp.results$'Variable Name'[i] <- rk.get.description (rk.temp.var, is.substitute=TRUE)
	if (rk.temp.options$dolength) try (rk.temp.results$'Length'[i] <- length (eval (rk.temp.var)))
	if (rk.temp.options$donacount) try (rk.temp.results$'NAs'[i] <- length (which(is.na(eval (rk.temp.var)))))
	rk.temp.test <- ad.test (eval (rk.temp.var))
	rk.temp.results$'Statistic'[i] <- paste (names (rk.temp.test$statistic), rk.temp.test$statistic, sep=" = ")
	rk.temp.results$'p-value'[i] <- rk.temp.test$p.value
}

<?
}

function printout () {
?>
rk.header ("Anderson-Darling Normality Test")

rk.results (rk.temp.results)
<?
}

function cleanup () {

?>
rm (rk.temp.results)
rm (rk.temp.options)
rm (rk.temp.var)
rm (rk.temp.test)
<?
}
?>