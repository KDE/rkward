function calculate () {
	echo ('result <- var.test (' + getValue ("x") + ', ' + getValue ("y") + ', alternative = "' + getValue ("alternative") + '", ratio = ' + getValue ("ratio"));
	var conflevel = getValue ("conflevel");
	if (conflevel != "0.95") echo (", conf.level=" + conflevel);
	echo (')\n');
	echo ('\n');
}

function printout () {
	echo ('names <- rk.get.description (' + getValue ("x") + ', ' + getValue ("y") + ')\n');
	echo ('\n');
	echo ('rk.header (result$method,\n');
	echo ('	parameters=list ("Confidence Level", "' + getValue ("conflevel") + '", "Alternative Hypothesis", rk.describe.alternative(result)))\n');
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	\'Variables\'=names,\n');
	echo ('	\'F\'=result$statistic["F"],\n');
	echo ('	\'Numerator DF\'=result$parameter["num df"],\n');
	echo ('	\'Denominator DF\'=result$parameter["denom df"],\n');
	echo ('	\'p-value\'=result$p.value,\n');
	echo ('	\'Lower CI\'=result$conf.int[1],\n');
	echo ('	\'Upper CI\'=result$conf.int[2],\n');
	echo ('	\'ratio of variances\'=result$estimate))\n');
}

