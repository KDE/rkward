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
?>cat ("<h1>T-test (independent samples)</h1>\n")
cat (paste ("<h2>Comparing", deparse (rk.temp.x), "against", deparse (rk.temp.y), "</h2>\n"))
if (rk.temp$alternative == "less") {
	cat (paste ("<h3>H1:", rk.get.short.name (rk.temp.y), "is greater than", rk.get.short.name (rk.temp.x), "<h3>"))
} else if (rk.temp$alternative == "greater") {
	cat (paste ("<h3>H1:", rk.get.short.name (rk.temp.x), "is greater than", rk.get.short.name (rk.temp.y), "<h3>"))
} else {
	cat (paste ("<h3>H1:", rk.get.short.name (rk.temp.x), "and", rk.get.short.name (rk.temp.y), "differ<h3>"))
}
if (rk.temp.var.equal) {
	cat ("<h4>Assuming equal variances</h4>")
} else {
	cat ("<h4>Not assuming equal variances</h4>")
}
cat ("<table border=\"1\">")
cat ("<tr><td>Variable</td><td>estimated mean</td><td>degrees of freedom</td><td>t</td><td>p</td>")
if (rk.temp.print.conf.level) cat (paste ("<td>confidence interval of difference (", 100 * attr(rk.temp$conf.int, "conf.level"), "%)</td>", sep=""))
cat ("</tr>\n")
cat (paste ("<tr><td>", rk.get.description (rk.temp.x), "</td><td>", rk.temp$estimate[1], "</td><td rowspan=\"2\">", rk.temp$parameter, "</td><td rowspan=\"2\">", rk.temp$statistic, "</td><td rowspan=\"2\">", rk.temp$p.value, "</td>", sep=""))
if (rk.temp.print.conf.level) cat (paste ("<td rowspan=\"2\">[", rk.temp$conf.int[1], " .. ", rk.temp$conf.int[2],"]</td>", sep=""))
cat ("</tr>\n")
cat (paste ("<tr><td>", rk.get.description (rk.temp.y), "</td><td>", rk.temp$estimate[2], "</td><tr>", sep=""))
cat ("</table>")
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
