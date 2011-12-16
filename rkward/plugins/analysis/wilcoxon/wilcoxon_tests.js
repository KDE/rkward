function preprocess(){
	var x = getValue("x");
	var y = getValue("y");
	var ties = getValue("ties");
	if(ties) {
		echo("\trequire(exactRankTests)\n");
	} else {}
	echo("\tnames <- rk.get.description (" + x);
	if(y) {
		echo(", " + y);
	} else {}
	echo(")\n");
}

function calculate(){
	// read in variables from dialog
	var x = getValue("x");
	var y = getValue("y");
	var alternative = getValue("alternative");
	var paired = getValue("paired");
	var svbSvrsltst = getValue("svb_Svrsltst");
	var conflevel = getValue("conflevel");
	var mu = getValue("mu");
	var exact = getValue("exact");
	var correct = getValue("correct");
	var ties = getValue("ties");
	var confintChecked = getValue("confint.checked");

	// the R code to be evaluated
	var confintChecked = getValue("confint.checked");
	echo("\twcox.result <- ");
	if(ties) {
		echo("wilcox.exact(");
	} else {
		echo("wilcox.test(");
	}
	if(x) {
		echo("\n\t\tx=" + x);
	} else {}
	if(y) {
		echo(",\n\t\ty=" + y);
	} else {}
	if(alternative != "two.sided") {
		echo(",\n\t\talternative=\"" + alternative + "\"");
	} else {}
	if(mu != 0) {
		echo(",\n\t\tmu=" + mu);
	} else {}
	if(y && paired) {
		echo(",\n\t\tpaired=TRUE");
	} else {}
	if(exact != "automatic") {
		echo(",\n\t\texact=" + exact);
	} else {}
	if(correct) {
		echo(",\n\t\tcorrect=TRUE");
	} else {}
	if(confintChecked) {
		echo(",\n\t\tconf.int=TRUE");
		if(conflevel != 0.95) {
			echo(",\n\t\tconf.level=" + conflevel);
		} else {}
	} else {}
	echo("\n\t)\n\n");
}

function printout(){
	// printout the results


	var confintChecked = getValue("confint.checked");
	var correct = getValue("correct");
	var exact = getValue("exact");
	var paired = getValue("paired");
	var mu = getValue("mu");
	echo("rk.header (wcox.result$method,\n" + "\tparameters=list (\"Comparing\", paste (names, collapse=\" against \"),\n" + "\t\"H1\", rk.describe.alternative (wcox.result),\n" + "\t\"Continuity correction in normal approximation for p-value\", ");
	if(correct) {
		echo("\"TRUE\",\n");
	} else {
		echo("\"FALSE\",\n");
	}
	echo("\t\"Compute exact p-value\", \"" + exact + "\",\n");
	echo("\t\"Paired test\", ");
	if(paired) {
		echo("\"TRUE\",\n");
	} else {
		echo("\"FALSE\",\n");
	}
	echo("\t\"mu\", \"" + mu + "\"))\n\n");
	echo("rk.results (list (\n" + "\t\"Variable Names\"=names,\n" + "\t\"Statistic\"=wcox.result$statistic,\n" + "\t\"Location Shift\"=wcox.result$null.value,\n" + "\t\"Hypothesis\"=wcox.result$alternative,\n" + "\tp=wcox.result$p.value");
	if(confintChecked) {
		echo(",\n\t\"Confidence interval percent\"=(100 * attr(wcox.result$conf.int, \"conf.level\")),\n" + "\t\"Confidence interval of difference\"=wcox.result$conf.int,\n" + "\t\"Difference in Location\"=wcox.result$estimate");
	} else {}
	echo("))\n");
	//// save result object
	// read in saveobject variables
	var svbSvrsltst = getValue("svb_Svrsltst");
	var svbSvrsltstActive = getValue("svb_Svrsltst.active");
	var svbSvrsltstParent = getValue("svb_Svrsltst.parent");
	// assign object to chosen environment
	if(svbSvrsltstActive) {
		echo("assign(\"" + svbSvrsltst + "\", wcox.result, envir=" + svbSvrsltstParent + ")\n");
	} else {}

}

