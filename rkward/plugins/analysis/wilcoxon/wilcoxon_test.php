<?
function preprocess () { ?>
names = rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)
<?
}

function calculate () {
	$exact_setting = getRK_val ("exact");
	if ($exact_setting == "yes") {
		$exact_opt = ", exact=TRUE";
	} else if ($exact_setting == "no") {
		$exact_opt = ", exact=FALSE";
	}
?>
result <- wilcox.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>, alternative = "<? getRK ("alternative"); ?>", mu = <? getRK ("mu"); ?>, paired = <? getRK ("paired"); ?><? echo ($exact_opt); ?>, correct = <? getRK ("correct"); ?>, conf.int = <? getRK ("confint"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
rk.header (result$method,
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
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
