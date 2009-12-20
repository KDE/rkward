function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var data            = getValue("x");
	var chk_select      = getValue("chk_select");
	var inp_items       = getValue("inp_items");
	var spin_samples    = getValue("spin_samples");
	var chk_standard    = getValue("chk_standard");
	var chk_na          = getValue("chk_na");
	var chk_bsci        = getValue("chk_bsci");
	var spin_ci         = getValue("spin_ci");
	// reformat inp_items
	if (inp_items)
		inp_items   = inp_items.replace(/\n/g,', ').replace(/(\w*)\[\[|\]\]/g, '');

	///////////////////////////////////
	// check for selected options
	var cilo = "";
	var cihi = "";
	var options = new Array() ;
	if (chk_standard == "standard")
	  options[options.length] = "standardized=TRUE" ;
	if (chk_bsci == "bsci")
	  options[options.length] = "CI=TRUE" ;
	if (spin_ci != .95) {
	  cilo = (1-spin_ci)/2 ;
	  cihi = 1-cilo ;
	  options[options.length] = "probs=c("+cilo+", "+cihi+")" ;
	}
	if (spin_samples != 1000)
	  options[options.length] = "B="+spin_samples ;
	if (chk_na == "rm")
	  options[options.length] = "na.rm=TRUE" ;


	echo ('cronalpha.res <- cronbach.alpha(');
	if (data && chk_select && inp_items)
		echo ("subset("+data+", select=c("+inp_items+"))");
	else
		echo (data);
	// check if any advanced control options must be inserted
		if (options.length > 0) echo(", "+options.join(", "));
	echo (')\n');
	echo ('descript.res <- descript(');
		if (data && chk_select && inp_items)
			echo ("subset("+data+", select=c("+inp_items+"))");
		else
			echo (data);
	echo (', chi.squared=FALSE, B=' + spin_samples + ')\n');
}

function printout () {
	var chk_select      = getValue("chk_select");
	var spin_samples    = getValue("spin_samples");
	var chk_standard    = getValue("chk_standard");
	var chk_na          = getValue("chk_na");
	var chk_bsci        = getValue("chk_bsci");
	var spin_ci         = getValue("spin_ci");
	var inp_items       = getValue("inp_items");
	// reformat inp_items
	if (inp_items)
		inp_items   = inp_items.replace(/\n/g,', ').replace(/(\w*)\[\["|"\]\]/g, '');

	echo ('rk.header ("Cronbach\'s alpha")\n');
	echo ('rk.print("for the \'' + getValue("x") + '\' data-set');
	if (chk_select && inp_items)
		echo (" (subset: "+inp_items+")");
	echo ('")\n');
	echo ('rk.print(paste("Items:",cronalpha.res$p,"<br />Sample units:",cronalpha.res$n,"<br /><strong>alpha:",round(cronalpha.res$alpha, digits=2),"</strong>');
	if (chk_standard) echo(" (standardized)");
	echo ('"))\n');
	echo ('rk.print("Effects on alpha if items are removed:")\n');
	echo ('rk.print(descript.res$alpha)\n');
	if (chk_bsci) {
		echo ('rk.print("' + (spin_ci * 100)+"%" + ' Confidence interval:")\n');
		echo ('rk.print(cronalpha.res$ci)\n');
	}
}
