<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp <- t.test (eval (rk.temp.x), eval (rk.temp.y), "<? getRK ("hypothesis"); ?>"<?
if (getRK_val ("varequal")) echo (", var.equal=TRUE");
if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)
rk.temp.print.conf.level <- <? if (getRK_val ("confint")) echo "TRUE"; else echo "FALSE"; ?>

<?
	}
	
	function printout () {
?>
rk.header ("T-test (independent samples)", 
	parameters=list ("Comparing", paste (rk.get.description (rk.temp.x, is.substitute=TRUE), "against", rk.get.description (rk.temp.y, is.substitute=TRUE)),
	"H1", if (rk.temp$alternative == "less")
		paste (rk.get.short.name (rk.temp.y), "is greater than", rk.get.short.name (rk.temp.x))
	      else if (rk.temp$alternative == "greater")
		paste (rk.get.short.name (rk.temp.x), "is greater than", rk.get.short.name (rk.temp.y))
	      else
		paste (rk.get.short.name (rk.temp.x), "and", rk.get.short.name (rk.temp.y), "differ"),
	"Equal variances", "<? if (!getRK_val ("varequal")) echo "not"; ?> assumed"))

rk.results (list (
	'Variable Name'=rk.get.description (rk.temp.x, rk.temp.y, is.substitute=TRUE),
	'estimated mean'=rk.temp$estimate,
	'degrees of freedom'=rk.temp$parameter,
	t=rk.temp$statistic,
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
