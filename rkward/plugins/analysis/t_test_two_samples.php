<?
function preprocess () {
	global $x;
	global $y;

	$x = getRK_val ("x");
	$y = getRK_val ("y");
?>
names <- rk.get.description (<? echo ($x . ", " . $y); ?>)
<?
}

function calculate () {
	global $x;
	global $y;
	global $varequal;
	global $paired;

	$conflevel = getRK_val ("conflevel");
	$varequal = getRK_val ("varequal");
	$paired = getRK_val ("paired");
	$hypothesis = getRK_val ("hypothesis");

	$options = ", alternative=\"" . $hypothesis . "\"";
	if ($paired) $options .= ", paired=TRUE";
	if ((!$paired) && $varequal) $options .= ", var.equal=TRUE";
	if ($conflevel != "0.95") $options .= ", conf.level=" . $conflevel;
?>
result <- t.test (<? echo ($x . ", " . $y . $options); ?>)
<?
}

function printout () {
	global $varequal;
	global $paired;
?>
rk.header (result$method, 
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	"H1", rk.describe.alternative (result)<?
	if (!$paired) { ?>
,
	"Equal variances", "<? if (!$varequal) echo ("not"); ?> assumed"<?
	} ?>))

rk.results (list (
	'Variable Name'=names,
	'estimated mean'=result$estimate,
	'degrees of freedom'=result$parameter,
	t=result$statistic,
	p=result$p.value<?
	if (getRK_val ("confint")) { ?>,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int <? } ?>))
<?
}

?>
