/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess() {
	// we'll need the ltm package, so in case it's not loaded...
	echo('require(ltm)\n');
}

function calculate() {
}

function printout() {
	doPrintout(true);
}

function preview() {
	preprocess();
	calculate();
	doPrintout(false);
}

function doPrintout(full) {
	// this function takes care of generating the code for the printout() section. If full is set to true,
	// it generates the full code, including headers. If full is set to false, only the essentials will
	// be generated.

	// let's read all values into php variables for the sake of readable code
	var plot_type = getValue("plot_type");
	var plot_type_item = getValue("plot_type_item");
	var inp_items = getValue("inp_items");
	var spin_from = getValue("spin_from");
	var spin_to = getValue("spin_to");
	var annotation = getValue("annotation");
	var spin_categ = getValue("spin_categ");

	// in case there are generic plot options defined:
	var plot_options = getValue("plotoptions.code.printout");
	var plot_ops_main = getValue("plotoptions.main");
	var plot_ops_type = getValue("plotoptions.pointtype");
	var plot_ops_xlab = getValue("plotoptions.xlab");
	var plot_ops_ylab = getValue("plotoptions.ylab");

	///////////////////////////////////
	// check for selected options
	var arr_items = "";
	var options = new Array();
	if (plot_type == "items" && plot_type_item == "ICC")
		options[options.length] = "type=\"ICC\"";
	if (plot_type == "items" && plot_type_item == "IIC")
		options[options.length] = "type=\"IIC\"";
	if (plot_type == "items" && plot_type_item == "OCCu")
		options[options.length] = "type=\"OCCu\"";
	if (plot_type == "items" && plot_type_item == "OCCl")
		options[options.length] = "type=\"OCCl\"";
	// plot all items?
	if (plot_type == "items" && inp_items) {
		// for user convenience, we replace "-", ";" and space, split all input into an array
		// and join it again, separated by commas:
		arr_items = inp_items.replace(/-/g, ":").replace(/\s|;/g, ",");
		options[options.length] = "items=c(" + arr_items + ")";
	}
	// plot all categories?
	if (plot_type == "items" && spin_categ != 0)
		options[options.length] = "category=" + spin_categ;

	// for the test information curve, items must be set to "0":
	if (plot_type == "TIC")
		options[options.length] = "type=\"IIC\", items=0";
	// there is no option for standard error curves yet, so we need some extra magic
	// (see the "SEC" section in the plotting function below as well!)
	if (plot_type == "SEC")
		options[options.length] = "type=\"IIC\", items=0, plot=FALSE";

	// more advanced options
	// user defined zrange? we'll round it to two digits
	if (spin_from != -3.8 || spin_to != 3.8)
		options[options.length] = "zrange=c(" + (Math.round(spin_from * 100) / 100) + "," + (Math.round(spin_to * 100) / 100) + ")";
	// annotate lines and show legend?
	if (annotation == "legend")
		options[options.length] = "legend=TRUE";
	if (annotation == "plain")
		options[options.length] = "annot=FALSE";

	if (full) {
		new Header(i18n("Graded response model plot")).print();
		echo('\n');
		echo('rk.graph.on()\n');
	}
	// only the following section will be generated for $full==false
	echo('try({\n');
	printIndentedUnlessEmpty('\t', getValue("plotoptions.code.preprocess"), '', '\n');

	// first we'll check wheter standard error curves should be plotted,
	// because it takes two steps to draw them:
	if (plot_type == "SEC") {
		comment('two steps are needed to plot standard error curves', '\t');
		comment('first some values are generated...', '\t');
		echo('	res <- plot(' + getValue("x"));
		if (options.length > 0) echo(", " + options.join(", "));
		echo(')\n');
		echo('\n');
		comment('... and then they\'re used to plot the curves:', '\t');
		echo('	plot(res[,"z"], 1/sqrt(res[,"test.info"]), lwd=2');
		// we give come defaults, but they can be changed via the embedded plot options:
		if (!plot_ops_type) echo(", type=\"l\"");
		if (!plot_ops_xlab) echo(", xlab=" + i18nc("Item response theory", "Ability"));
		;
		if (!plot_ops_ylab) echo(", ylab=" + i18n("Standard Error"));
		if (!plot_ops_main) echo(", main=" + i18n("Standard Error of Measurement"));
		if (plot_options) echo(plot_options);
		echo(')\n');
	}
	// and this will be plotted if anything else than stadard error curves are chosen:
	else {
		echo('	plot(' + getValue("x"));
		if (options.length > 0) echo(", " + options.join(", "));
		if (plot_options) echo(plot_options);
		echo(')\n');
	}

	printIndentedUnlessEmpty('\t', getValue("plotoptions.code.calculate"), '\n', '');
	echo('})\n');

	if (full) echo('rk.graph.off()\n');
}
