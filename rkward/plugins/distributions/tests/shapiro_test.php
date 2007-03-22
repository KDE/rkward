<?
function preprocess () {
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv())
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
<?	} ?>
	try ({
		test <- shapiro.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
	})
}
<?
}

function printout () {
?>
rk.header ("Shapiro-Wilk Normality Test")
rk.results (results)
<?
}
?>