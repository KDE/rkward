// globals
var x;
var y;
var varequal;
var paired;

function preprocess () {
	x = getValue ("x");
	y = getValue ("y");

	echo ('names <- rk.get.description (' + x + ", " + y + ')\n');
}

function calculate () {
	varequal = getValue ("varequal");
	paired = getValue ("paired");

	var conflevel = getValue ("conflevel");
	var hypothesis = getValue ("hypothesis");

	var options = ", alternative=\"" + hypothesis + "\"";
	if (paired) options += ", paired=TRUE";
	if ((!paired) && varequal) options += ", var.equal=TRUE";
	if (conflevel != "0.95") options += ", conf.level=" + conflevel;

	echo ('result <- t.test (' + x + ", " + y + options + ')\n');
}

function printout () {
	echo ('rk.header (result$method, \n');
	echo ('	parameters=list ("Comparing", paste (names[1], "against", names[2]),\n');
	echo ('	"H1", rk.describe.alternative (result)');
	if (!paired) {
		echo (',\n');
		echo ('	"Equal variances", "');
		if (!varequal) echo ("not");
		echo (' assumed"');
	}
	echo ('))\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	\'Variable Name\'=names,\n');
	echo ('	\'estimated mean\'=result$estimate,\n');
	echo ('	\'degrees of freedom\'=result$parameter,\n');
	echo ('	t=result$statistic,\n');
	echo ('	p=result$p.value');
	if (getValue ("confint")) {
		echo (',\n');
		echo ('	\'confidence interval percent\'=(100 * attr(result$conf.int, "conf.level")),\n');
		echo ('	\'confidence interval of difference\'=result$conf.int ');
	}
	echo ('))\n');
}


