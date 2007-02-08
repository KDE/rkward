<?
function preprocess () {
}

function calculate () {
	$exact_setting = getRK_val ("exact");
	if ($exact_setting == "yes") {
		$exact_opt = ", exact=TRUE";
	} else if ($exact_setting == "no") {
		$exact_opt = ", exact=FALSE";
	}
?>
require(exactRankTests)
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- ansari.exact (eval (rk.temp.x), eval (rk.temp.y), alternative = "<? getRK ("alternative"); ?>"<? echo ($exact_opt); ?>, conf.int = <? getRK ("confint"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
}

function printout () {
?>
rk.header ("Ansari-Bradley two-sample exact test", 
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "against", rk.get.description (rk.temp.y, is.substitute=TRUE)),"Confidence Interval", "<? getRK ("confint"); ?>", "Confidence Level",<? getRK ("conflevel"); ?>, "Compute exact p-value", "<? getRK ("exact"); ?>"))

rk.results (list (
	'Variable Names'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'statistic'=rk.temp$statistic,
	'null.value'=rk.temp$null.value,
	'Hypothesis'=rk.temp$alternative,
	p=rk.temp$p.value<?
	if (getRK_val ("confint")== "TRUE") { ?>,
	'confidence interval percent'=(100 * attr(rk.temp$conf.int, "conf.level")),
	'confidence interval of difference'=rk.temp$conf.int <? } ?>))
<?
}

function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
?>
