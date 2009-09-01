<?
function preprocess () {
}

function calculate () {
}

function printout () {
	if (getRK_val("custom") == 0)
		$lambda = getRK_val ("lambda");
	else
		$lambda = getRK_val ("clambda");
?>
rk.header ("Hodrick-Prescott Filter", parameters=list("Lambda", <? echo $lambda; ?>))
x <- get("<? getRK("x"); ?>", envir=globalenv())
lambda <- <? echo $lambda . "\n"; ?>

if (any (is.na (x))) stop ("Missing values cannot be handled")

i <- diag(length(x))
trend <- solve(i + lambda * crossprod(diff(i, lag=1, d=2)), x) # The HP Filter itself. Thanks to Grant V. Farnsworth
cycle <- x - trend
if (is.ts(x)) {
	trend <- ts(trend,start(x),frequency=frequency(x))
	cycle <- ts(cycle,start(x),frequency=frequency(x))
}
<?
	if (getRK_val("create_trend") == 1) {
?>
assign("<? getRK("trend_name"); ?>", trend, envir=globalenv())
<?
	} 
	if (getRK_val("create_cycle") == 1) {
?>
assign("<? getRK("cycle_name"); ?>", cycle, envir=globalenv())
<?
	}

	if (getRK_val("series_col.color") != "" & getRK_val("trend_col.color") != "")
		$upcol = ", col=c(\"" . getRK_val("series_col.color") . "\", \"" . getRK_val("trend_col.color") . "\")";
	elseif (getRK_val("series_col.color") != "")
		$upcol = ", col=c(\"" . getRK_val("series_col.color") . "\", \"black\")";
	elseif (getRK_val("trend_col.color") != "")
		$upcol = ", col=c(\"black\", \"" . getRK_val("trend_col.color") . "\")";
	else
		$upcol = "";

	if (getRK_val("series_lty") != "" & getRK_val("trend_lty") != "")
		$uplty = ", lty=c(\"" . getRK_val("series_lty") . "\", \"" . getRK_val("trend_lty") . "\")";
	elseif (getRK_val("series_lty") != "")
		$uplty = ", lty=c(\"" . getRK_val("series_lty") . "\", \"solid\")";
	elseif (getRK_val("trend_lty") != "")
		$uplty = ", lty=c(\"solid\", \"" . getRK_val("trend_lty") . "\")";
	else
		$uplty = "";

	if (getRK_val("uplab.text") == "")
		$uplab = "\"" . getRK_val("x") . ", Trend\"";
	else
		if (getRK_val("uplabisquote") == 1)		
			$uplab = "\"" . getRK_val("uplab") . "\"";
		else
			$uplab = getRK_val("uplab");
?>
rk.graph.on ()
try({
	par(mfrow=c(<?if (getRK_val("plot_cycle") == 1) echo 2; else echo 1;?>,1),mar=c(2,4,2,2)+0.1)
	plot.ts(cbind(x, trend), ylab=<? echo $uplab; echo $upcol; ?>,lwd=c(<? getRK("series_lwd"); ?>,<? getRK("trend_lwd"); ?>)<? echo $uplty; ?>, plot.type="single")
<?
	if (getRK_val("plot_cycle") == 1) {
		if (getRK_val("downlab.text") == "") 
			$downlab = "\"Cycle\"";
		else
			if (getRK_val("downlabisquote") == 1)	
				$downlab = "\"" . getRK_val("downlab") . "\"";
			else
				$downlab = getRK_val("downlab");
?>
	plot.ts(cycle, ylab=<? echo $downlab; if (getRK_val("cycle_col.color") != "") echo ", col=\"" . getRK_val("cycle_col.color") . "\""; ?>, lwd=<? getRK("cycle_lwd"); if (getRK_val("cycle_lty") != "") echo ", lty=\"" . getRK_val("cycle_lty") . "\""; ?>)
<?
	}
?>
})
rk.graph.off ()
<?
}
?>
