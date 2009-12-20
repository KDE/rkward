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
	var inp_items        = getValue("inp_items");
	var spin_abilfrom    = getValue("spin_abilfrom");
	var spin_abilto      = getValue("spin_abilto");
	var spin_probfrom    = getValue("spin_probfrom");
	var spin_probto      = getValue("spin_probto");
	var annotation       = getValue("annotation");
	var chk_ask          = getValue("chk_ask");
	var chk_mplot        = getValue("chk_mplot");

	// in case there are generic plot options defined:
	var plot_options     = getValue("plotoptions.code.printout");
	var plot_ops_main    = getValue("plotoptions.main");
	var plot_ops_type    = getValue("plotoptions.pointtype");
	var plot_ops_xlab    = getValue("plotoptions.xlab");
	var plot_ops_ylab    = getValue("plotoptions.ylab");

	///////////////////////////////////
	// check for selected options
	var arr_items = "";
	var options = new Array() ;
	// plot all items?
	if (inp_items) {
		// for user convenience, we replace "-", ";" and space, split all input into an array
		// and join it again, separated by commas:
		arr_items = inp_items.replace(/-/g,":").replace(/\s|;/g,",");
		options[options.length] = "item.subset=c("+arr_items+")";
	}
	if (chk_mplot == "mplot")
		options[options.length] = "mplot=TRUE" ;
	if (chk_ask != "ask")
		options[options.length] = "ask=FALSE" ;

	// more advanced options
	// user defined ranges? we'll round it to two digits
	if ((spin_abilfrom != -4 || spin_abilto != 4) && spin_abilfrom < spin_abilto)
		options[options.length] = "xlim=c("+(Math.round(spin_abilfrom*100)/100)+","+(Math.round(spin_abilto*100)/100)+")" ;
	if ((spin_probfrom != 0 || spin_probto != 1) && spin_probfrom < spin_probto)
		options[options.length] = "ylim=c("+(Math.round(spin_probfrom*100)/100)+","+(Math.round(spin_probto*100)/100)+")" ;
	// annotate lines and show legend?
	if (annotation == "plain")
		options[options.length] = "legpos=FALSE" ;

	if (full) {
		echo ('rk.header("Partial credit model plot")\n');
		echo ('\n');
		echo ('rk.graph.on()\n');
	}
	// only the following section will be generated for full==false

	echo ('try(plotICC(' + getValue("x"));
	if (options.length > 0) echo(", "+options.join(", "));
	if (plot_options) echo(plot_options);
	echo ('))\n');

	if (full) echo ('rk.graph.off()\n');
}
