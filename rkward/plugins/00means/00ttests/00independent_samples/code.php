<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp.x <- substitute (<? getRK ("x"); ?>)
rk.temp.y <- substitute (<? getRK ("y"); ?>)
rk.temp.var.equal <- <? getRK ("varequal"); ?> 
rk.temp <- t.test (eval (rk.temp.x), eval (rk.temp.y), "<? getRK ("hypothesis"); ?>", var.equal=rk.temp.var.equal<?; if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)
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
	"Equal variances:", if (rk.temp.var.equal) "assumed" else "not assumed"))

rk.results (list (
	rk.get.description (rk.temp.x),
	rk.temp$estimate,
	rk.temp$parameter,
	rk.temp$statistic,
	rk.temp$p.value,
	if (rk.temp.print.conf.level) rk.temp$conf.int),
	titles=c ("Variable",
	"estimated mean",
	"degrees of freedom",
	"t",
	"p",
	if (rk.temp.print.conf.level) paste ("confidence interval of difference (", 100 * attr(rk.temp$conf.int, "conf.level"), "%)")))
<?
	}
	
	function cleanup () {
?>rm (rk.temp)
rm (rk.temp.print.conf.level)
rm (rk.temp.var.equal)
rm (rk.temp.x)
rm (rk.temp.y)
<?
	}
?>
