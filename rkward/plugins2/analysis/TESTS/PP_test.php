<?
function preprocess () {
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
objects <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)
	var <- eval (objects[[i]], envir=globalenv ())
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

<?	}
	if (getRK_val ("narm")) { ?>
	var <- var[!is.na (var)] 	# remove NAs
<?	} ?>
	try ({
		test <- PP.test (var, lshort = <? getRK ("lshort"); ?>)
		results[i, 'Dickey-Fuller'] <- test$statistic
		results[i, 'Truncation lag parameter'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
<?
}

function printout () {
?>
rk.header ("Phillips-Perron Test for Unit Roots",
	parameters=list ("Truncation lag parameter short ('TRUE') or long ('FALSE')", "<? getRK ("lshort"); ?>"))

rk.results (results)
<?
}
?>
