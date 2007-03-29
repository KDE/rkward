<?
function preprocess () { ?>
require(exactRankTests)

names <- rk.get.description (<? getRK ("x"); ?>, <? getRK ("y"); ?>)
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
result <- ansari.exact (<? getRK ("x"); ?>, <? getRK ("y"); ?>, alternative = "<? getRK ("alternative"); ?>"<? echo ($exact_opt); ?>, conf.int = <? getRK ("confint"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
rk.header (result$method,
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	'H1', rk.describe.alternative (result),
	"Compute exact p-value", "<? getRK ("exact"); ?>"<? if (getRK_val ("confint")== "TRUE") {?>,
	"Confidence Level", "<? getRK ("conflevel"); ?>" <?}?>))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'null.value'=result$null.value,
	p=result$p.value<?
	if (getRK_val ("confint")== "TRUE") { ?>,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int,
	'estimate of the ratio of scales'=result$estimate<? } ?>))
<?
}
?>
