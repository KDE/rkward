function printout () {
	var lambda = "";
	if (getValue("custom") == 0) {
		lambda = getValue ("lambda");
	} else {
		lambda = getValue ("clambda");
	}

	echo ('rk.header ("Hodrick-Prescott Filter", parameters=list("Lambda", ' +  lambda + '))\n');
	echo ('x <- get("' + getValue("x") + '", envir=globalenv())\n');
	echo ('lambda <- ' +  lambda + "\n");
	echo ('\n');
	echo ('if (any (is.na (x))) stop ("Missing values cannot be handled")\n');
	echo ('\n');
	echo ('i <- diag(length(x))\n');
	echo ('trend <- solve(i + lambda * crossprod(diff(i, lag=1, d=2)), x) # The HP Filter itself. Thanks to Grant V. Farnsworth\n');
	echo ('cycle <- x - trend\n');
	echo ('if (is.ts(x)) {\n');
	echo ('	trend <- ts(trend,start(x),frequency=frequency(x))\n');
	echo ('	cycle <- ts(cycle,start(x),frequency=frequency(x))\n');
	echo ('}\n');
	if (getValue("create_trend") == 1) {
		echo ('assign("' + getValue("trend_name") + '", trend, envir=globalenv())\n');
	}
	if (getValue("create_cycle") == 1) {
		echo ('assign("' + getValue("cycle_name") + '", cycle, envir=globalenv())\n');
	}

	var upcol = "";
	if (getValue("series_col.color") != "" & getValue("trend_col.color") != "") {
		upcol = ", col=c(\"" + getValue("series_col.color") + "\", \"" + getValue("trend_col.color") + "\")";
	} else if (getValue("series_col.color") != "") {
		upcol = ", col=c(\"" + getValue("series_col.color") + "\", \"black\")";
	} else if (getValue("trend_col.color") != "") {
		upcol = ", col=c(\"black\", \"" + getValue("trend_col.color") + "\")";
	} else {
		upcol = "";
	}

	var uplty = "";
	if (getValue("series_lty") != "" & getValue("trend_lty") != "") {
		uplty = ", lty=c(\"" + getValue("series_lty") + "\", \"" + getValue("trend_lty") + "\")";
	} else if (getValue("series_lty") != "") {
		uplty = ", lty=c(\"" + getValue("series_lty") + "\", \"solid\")";
	} else if (getValue("trend_lty") != "") {
		uplty = ", lty=c(\"solid\", \"" + getValue("trend_lty") + "\")";
	} else {
		uplty = "";
	}

	var uplab = "";
	if (getValue("uplab.text") == "") {
		uplab = "\"" + getValue("x") + ", Trend\"";
	} else if (getValue("uplabisquote") == 1) {
		uplab = "\"" + getValue("uplab") + "\"";
	} else {
		uplab = getValue("uplab");
	}

	echo ('rk.graph.on ()\n');
	echo ('try({\n');
	echo ('	par(mfrow=c(');
	if (getValue("plot_cycle") == 1) echo ("2");
	else echo ("1");
	echo (',1),mar=c(2,4,2,2)+0.1)\n');
	echo ('	plot.ts(cbind(x, trend), ylab=' +  uplab +  upcol + ',lwd=c(' + getValue("series_lwd") + ',' + getValue("trend_lwd") + ')' +  uplty + ', plot.type="single")\n');
	var downlab = "";
	if (getValue("plot_cycle") == 1) {
		if (getValue("downlab.text") == "") {
			downlab = "\"Cycle\"";
		} else if (getValue("downlabisquote") == 1) {
			downlab = "\"" + getValue("downlab") + "\"";
		} else {
			downlab = getValue("downlab");
		}

		echo ('	plot.ts(cycle, ylab=' +  downlab);
		if (getValue("cycle_col.color") != "") echo (", col=\"" + getValue("cycle_col.color") + "\"");
		echo (', lwd=' + getValue("cycle_lwd"));
		if (getValue("cycle_lty") != "") echo (", lty=\"" + getValue("cycle_lty") + "\"");
		echo (')\n');
	}

	echo ('})\n');
	echo ('rk.graph.off ()\n');
}

