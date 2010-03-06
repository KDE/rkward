<?
function preprocess () { ?>
require(tseries)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
<?	} 
	if (getRK_val ("excludenas")) { ?>
	var <- na.omit (var)
<?	} ?>
	try ({
		test <- jarque.bera.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'df'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
<?
}

function printout () {
?>
rk.header ("Jarque-Bera Normality Test", parameters=list ("Exclude NAs", <? if (getRK_val ("excludenas")) echo "\"YES\""; else echo "\"NO\""; ?>))
rk.results (results)
<?
}
?>