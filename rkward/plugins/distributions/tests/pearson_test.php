<?
function preprocess () { ?>
require(nortest)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
	$adjust =  getRK_val ("adjust");

?>
vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
<?	}
	if (getRK_val ("nacount")) { ?>
	results[i, 'NAs'] <- length (which(is.na(var)))
<?	} ?>
	try ({
		test <- pearson.test (var, <? echo $adjust; ?>)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
		results[i, 'number of classes'] <- test$n.classes
		results[i, 'degrees of freedom'] <- test$df
	})
}
<?
}

function printout () {
?>
rk.header ("Pearson chi-square Normality Test",
	parameters=list ("chi-square distribution with n.classes-3 df (TRUE) or chi-square distribution with n.classes-1 df (FALSE)", "<? getRK ("adjust"); ?>"))
rk.results (results)
<?
}
?>