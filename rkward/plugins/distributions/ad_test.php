<?
function preprocess () {
}

function calculate () {
$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
require(nortest)

rk.temp.options <- list (dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)

rk.temp.vars <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame (object=rep (NA, length (rk.temp.vars)))

i=0;
for (rk.temp.var in rk.temp.vars) {
	i = i+1
	rk.temp.results$object[i] <- rk.get.description (rk.temp.var, is.substitute=TRUE)
	if (rk.temp.options$dolength) try (rk.temp.results$length[i] <- length (eval (rk.temp.var)))
	if (rk.temp.options$donacount) try (rk.temp.results$nacount[i] <- length (which(is.na(eval (rk.temp.var)))))
	rk.temp.test <- ad.test (eval (rk.temp.var))
	rk.temp.results$statistic[i] <- paste (names (rk.temp.test$statistic), rk.temp.test$statistic, sep=" = ")
	rk.temp.results$p.value[i] <- rk.temp.test$p.value
}

<?
}

function printout () {
?>
rk.header ("Anderson-Darling Normality Test")

rk.results (rk.temp.results, c ("Variable Name", if (rk.temp.options$dolength) "Length", if (rk.temp.options$donacount) "NAs", "Statistic", "p-value"))
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