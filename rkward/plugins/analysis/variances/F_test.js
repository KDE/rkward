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
	new Header (noquote ('result$method'))
	    .addFromUI ("conflevel")
	    .addFromUI ("ratio")
	    .add (i18n ("Alternative Hypothesis"), noquote ('rk.describe.alternative(result)')).print ();
	echo ('\n');
	echo ('rk.results (list (\n');
	echo ('	' + i18n ("Variables") + '=names,\n');
	echo ('	\'F\'=result$statistic["F"],\n');
	echo ('	' + i18n ("Numerator DF") + '=result$parameter["num df"],\n');
	echo ('	' + i18n ("Denominator DF") + '=result$parameter["denom df"],\n');
	echo ('	' + i18n ("p-value") + '=result$p.value,\n');
	echo ('	' + i18n ("Lower CI") + '=result$conf.int[1],\n');
	echo ('	' + i18n ("Upper CI") + '=result$conf.int[2],\n');
	echo ('	' + i18n ("ratio of variances") + '=result$estimate))\n');
}

