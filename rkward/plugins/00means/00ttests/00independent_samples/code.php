<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp <- t.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>, "<? getRK ("hypothesis"); ?>"<? if (($varequal = getRK_val ("varequal")) != "") echo (", var.equal=" . $varequal); if (($conflevel = getRK_val ("conflevel")) != "0.95") echo (", conf.level=" . $conflevel); ?>)
<?
	}
	
	function printout () {
		$x = getRK_val ("x.shortname");
		$y = getRK_val ("x.shortname");
	
		// fetch further values from RK
		if ($confint = getRK_val ("confint")) {
			$conflevel = getRK_val ("conflevel");
			$conflevel = ($conflevel * 100) . "%";
		}
		$varequal = "assuming equal variances";
		if (getRK_val ("varequal") == "") {
			$varequal = "not " . $varequal;
		}
		$hypothesis = getRK_val ("hypothesis");
		if ($hypothesis == "two.sided") {
			$hypothesis = $x . " and " . $y . " differ (two sided)";
		} else if ($hypothesis == "less") {
			$hypothesis = $y . " is greater than " . $x;
		} else {
			$hypothesis = $x . " is greater than " . $y;
		}
		
		// get rid of superflous information / formatting
		list ($dummy, $t) = explode ("\n", $t, 2);
		$t = trim ($t);
		list ($dummy, $df) = explode ("\n", $df, 2);
		$df = trim ($df);
		list ($dummy, $p) = explode (" ", $p, 2);
		$p = trim ($p);
		list ($dummy, $est) = explode ("\n", $est, 2);
		list ($estx, $esty) = explode (" ", trim ($est), 2);
		$estx = trim ($estx);
		$esty = trim ($esty);

		// produce the output
?>
cat ("<h1>T-test (independent samples)</h1>")
cat ("<h2>Comparing <? getRK ("x.label"); ?> against <? getRK ("x.label"); ?></h2>")
cat ("<h3>H1: <? echo ($hypothesis); ?><h3>")
cat ("<h4>(<? echo ($varequal); ?>)</h4>")
cat ("<table border=\"1\">")
cat (paste ("<tr><td>Variable</td><td>estimated mean</td><td>degrees of freedom</td><td>t</td><td>p</td><? if ($confint) { ?><td>confidence interval of difference (", 100 * attr(rk.temp$conf.int, "conf.level"), "%)</td><? } ?></tr>\n", sep=""))
cat (paste ("<tr><td><? getRK ("x.label"); ?></td><td>", rk.temp$estimate[1], "</td><td rowspan=\"2\">", rk.temp$parameter, "</td><td rowspan=\"2\">", rk.temp$statistic, "</td><td rowspan=\"2\">", rk.temp$p.value, "</td><? if ($confint) { ?><td rowspan=\"2\">[", rk.temp$conf.int[1], " .. ", rk.temp$conf.int[2], "]</td><? }?></tr>\n", sep=""))
cat (paste ("<tr><td><? getRK ("x.label"); ?></td><td>", rk.temp$estimate[2], "</td><tr>", sep=""))
cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp)
<?
	}
?>
