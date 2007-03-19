<?
function preprocess () { ?>
require (nortest)
<?
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
<?	}
	if (getRK_val ("nacount")) { ?>
	results[i, 'NAs'] <- length (which(is.na(var)))
<?	} ?>
	try ({
		test <- lillie.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
	})
}
<?
}

function printout () {
?>
rk.header ("Lilliefors (Kolmogorov-Smirnov) Normality test")
rk.results (results)
<?
}
?>