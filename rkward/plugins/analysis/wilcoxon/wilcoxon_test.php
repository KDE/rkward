<?
	function preprocess () {
	}
	
	function calculate () {
?>
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- wilcox.test (eval (rk.temp.x), eval (rk.temp.y), alternative = c("<? getRK ("alternative"); ?>"), mu = <? getRK ("mu"); ?>, paired = <? getRK ("paired"); ?>, exact = <? getRK ("exact"); ?>, correct = <? getRK ("correct"); ?>, conf.int = <? getRK ("confint"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)

<?
	}
	
	function printout () {
?>
rk.header ("Wilcoxon Test",
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "against", rk.get.description (rk.temp.y, is.substitute=TRUE)),
	"H1", if (rk.temp$alternative == "less")
		paste (rk.get.short.name (rk.temp.y), "is greater than", rk.get.short.name (rk.temp.x))
	      else if (rk.temp$alternative == "greater")
		paste (rk.get.short.name (rk.temp.x), "is greater than", rk.get.short.name (rk.temp.y))
	      else
		paste (rk.get.short.name (rk.temp.x), "and", rk.get.short.name (rk.temp.y), "differ"),"Compute Confidence Interval", "<? getRK ("confint"); ?>", "Continuity correction in normal approximation for p-value", "<? getRK ("correct"); ?>", "Compute exact p-value", "<? getRK ("exact"); ?>","mu", "<? getRK ("mu"); ?>"))

rk.results (list (
	'Variable Names'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'statistic'=rk.temp$statistic,
	'Location Shift'=rk.temp$null.value,
	'Hypothesis'=rk.temp$alternative,
	p=rk.temp$p.value<?
	if (getRK_val ("confint")== "TRUE") { ?>,
	'confidence interval percent'=(100 * attr(rk.temp$conf.int, "conf.level")),
	'confidence interval of difference'=rk.temp$conf.int,
	'Difference in Location' = rk.temp$estimate <? } ?>))
<?
	}
	
	function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
	}
?>
