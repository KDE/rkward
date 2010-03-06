<?
function preprocess () {
}

function calculate () {
	$vars = str_replace ("\n", " + ", trim (getRK_val ("x")));
	if (!getRK_val ("intercept.state.numeric")) $intercept = "0 + ";
?>
results <- summary.lm (lm (<? getRK ("y"); ?> ~ <? echo ($intercept . $vars); ?>))
<?
}

function printout () {
?>
rk.header ("Linear Regression")
rk.print(results)
<?
}
?>
