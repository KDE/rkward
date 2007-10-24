<?
function preprocess () { ?>
require(moments)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>

objects <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)

	try (results[i, 'Moment'] <- moment (var, order = <? getRK ("order"); ?>, central = <? getRK ("central"); ?>, absolute = <? getRK ("absolute"); ?>, na.rm = <? getRK ("narm"); ?>))
<?	if (getRK_val ("length")) { ?>

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
<?	} ?>
}
<?
}

function printout () {
?>
rk.header ("Statistical Moment",
	parameters=list ("Order", "<? getRK ("order"); ?>, "Compute central moments", "<? getRK ("central"); ?>", "Compute absolute moments", "<? getRK ("absolute"); ?>", "Remove missing values", "<? getRK ("narm"); ?>"))
rk.results (results)
<?
}
?>
