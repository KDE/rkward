<?
function preprocess () {
	global $y;
	$y = getRK_val ("y");
	if ($y != "") $y = ", " . $y;
?>
names = rk.get.description (<? getRK ("x"); echo ($y); ?>)
<?
}

function calculate () {
	global $y;

	$exact_setting = getRK_val ("exact");
	if ($exact_setting == "yes") {
		$exact_opt = ", exact=TRUE";
	} else if ($exact_setting == "no") {
		$exact_opt = ", exact=FALSE";
	}
	if ($y != "") $paired_opt = ", paired = " . getRK_val ("paired");
	if (getRK_val ("confint") == "TRUE") {
		if (($conflevel = getRK_val("conflevel")) != "0.95") $conflevel_opt = ", conf.level=" . $conflevel;
	}
?>
result <- wilcox.test (<? getRK ("x"); echo ($y); ?>, alternative = "<? getRK ("alternative"); ?>", mu = <? getRK ("mu"); echo ($paired_opt); echo ($exact_opt); ?>, correct = <? getRK ("correct"); ?>, conf.int = <? getRK ("confint"); echo ($conflevel_opt); ?>)

<?
}

function printout () {
?>
rk.header (result$method,
	parameters=list ("Comparing", paste (names, collapse=" against "),
	"H1", rk.describe.alternative (result),
	"Continuity correction in normal approximation for p-value", "<? getRK ("correct"); ?>",
	"Compute exact p-value", "<? getRK ("exact"); ?>", "Paired test", "<? getRK ("paired"); ?>",
	"mu", "<? getRK ("mu"); ?>"))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'Location Shift'=result$null.value,
	'Hypothesis'=result$alternative,
	p=result$p.value<?
	if (getRK_val ("confint") == "TRUE") { ?>,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int,
	'Difference in Location' = result$estimate<? } ?>))
<?
}
?>
