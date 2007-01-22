<?
	function preprocess () {
	}
	
	function calculate () {
?>
require(exactRankTests)
rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- wilcox.exact (eval (rk.temp.x), eval (rk.temp.y), alternative = c("<? getRK ("hypothesis"); ?>"), mu = <? getRK ("mu"); ?>, paired = <? getRK ("paired"); ?>, exact = <? getRK ("exact"); ?>, correct = <? getRK ("correct"); ?>, conf.int = <? getRK ("confint"); ?> <?
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)
rk.temp.print.conf.level <- <? if (getRK_val ("confint")) echo "TRUE"; else echo "FALSE"; ?>

<?
	}
	
	function printout () {
?>
rk.header ("Wilcoxon exact test", 
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "against", rk.get.description (rk.temp.y, is.substitute=TRUE)),
	"H1", if (rk.temp$alternative == "less")
		paste (rk.get.short.name (rk.temp.y), "is greater than", rk.get.short.name (rk.temp.x))
	      else if (rk.temp$alternative == "greater")
		paste (rk.get.short.name (rk.temp.x), "is greater than", rk.get.short.name (rk.temp.y))
	      else
		paste (rk.get.short.name (rk.temp.x), "and", rk.get.short.name (rk.temp.y), "differ")))

rk.results (list (
	'Variable Name'=rk.get.description (rk.temp.x),
	'statistic'=rk.temp$statistic,
	'degrees of freedom'=rk.temp$parameter,
	'mu'=rk.temp$null.value,
	'alternative'=rk.temp$alternative,
	p=rk.temp$p.value<?
	if (getRK_val ("confint")) { ?>,
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
