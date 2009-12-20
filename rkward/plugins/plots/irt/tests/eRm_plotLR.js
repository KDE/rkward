function preprocess () {
	// we'll need the eRm package, so in case it's not loaded...
	echo ('require(eRm)\n');
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	// this function takes care of generating the code for the printout() section. If full is set to true,
	// it generates the full code, including headers. If full is set to false, only the essentials will
	// be generated.

	// let's read all values into php variables for the sake of readable code
	var rad_splitcr      = getValue("rad_splitcr");
	var splitvector      = getValue("splitvector");
	var inp_items        = getValue("inp_items");
	var chk_se           = getValue("chk_se");
	var chk_confint      = getValue("chk_confint");
	var spin_confint     = getValue("spin_confint");
	var chk_ctrline      = getValue("chk_ctrline");
	var spin_ctrline     = getValue("spin_ctrline");
	var spin_abilfrom    = getValue("spin_abilfrom");
	var spin_abilto      = getValue("spin_abilto");
	var annotation       = getValue("annotation");

	// in case there are generic plot options defined:
	var plot_options     = getValue("plotoptions.code.printout");
	var plot_ops_main    = getValue("plotoptions.main");
	var plot_ops_type    = getValue("plotoptions.pointtype");
	var plot_ops_xlab    = getValue("plotoptions.xlab");
	var plot_ops_ylab    = getValue("plotoptions.ylab");

	///////////////////////////////////
	// check for selected options
	// these two arrays will contain the options for the two functions that will be called:
	var arr_items = "";
	var options_lrtest = new Array() ;
	var options_plotgof = new Array() ;
	// plot all items?
	if (inp_items) {
		// for user convenience, we replace "-", ";" and space, split all input into an array
		// and join it again, separated by commas:
		arr_items = inp_items.replace(/-/g,":").replace(/\s|;/g,",");
		options_plotgof[options_plotgof.length] = "beta.subset=c("+arr_items+")";
	}
	if (rad_splitcr == "mean" || rad_splitcr == "all.r")
		options_lrtest[options_lrtest.length] = "splitcr=\""+rad_splitcr+"\"";
	if (rad_splitcr == "vector" && splitvector)
	        options_lrtest[options_lrtest.length] = "splitcr="+splitvector;
	if (chk_se == "se")
	        options_lrtest[options_lrtest.length] = "se=TRUE";
	if (chk_confint == "conf") {
		if (spin_confint != 0.95)
			options_plotgof[options_plotgof.length] = "conf=list(gamma="+(Math.round(spin_confint*100)/100)+", col=\"red\", lty=\"dashed\", ia=FALSE)";
		else
			options_plotgof[options_plotgof.length] = "conf=list()";
		}
	if (chk_ctrline == "ctrline") {
		if (spin_ctrline != 0.95)
			options_plotgof[options_plotgof.length] = "ctrline=list(gamma="+(Math.round(spin_ctrline*100)/100)+", col=\"blue\", lty=\"solid\")";
		else
			options_plotgof[options_plotgof.length] = "ctrline=list()";
	}

	// more advanced options
	// user defined ranges? we'll round it to two digits
	if ((spin_abilfrom != -3 || spin_abilto != 3) && spin_abilfrom < spin_abilto)
		options_plotgof[options_plotgof.length] = "xlim=c("+(Math.round(spin_abilfrom*100)/100)+","+(Math.round(spin_abilto*100)/100)+")" ;
	// annotate lines and show legend?
	if (annotation == "number" || annotation == "none" || annotation == "identify")
		options_plotgof[options_plotgof.length] = "tlab=\""+annotation+"\"" ;

	if (full) {
		echo ('rk.header("Andersen\'s LR test")\n');
		echo ('\n');
		echo ('rk.graph.on()\n');
	}
	// only the following section will be generated for full==false

	echo ('lr.res <- LRtest(' + getValue("x"));
	if (options_lrtest.length > 0) echo(", "+options_lrtest.join(", "));
	echo (')\n');
	echo ('try(plotGOF(lr.res');
	if (options_plotgof.length > 0) echo(", "+options_plotgof.join(", "));
	if (plot_options.length > 0) echo(plot_options);
	echo ('))\n');

	if (full) echo ('rk.graph.off()\n');
}
