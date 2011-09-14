// globals
var options;

function preprocess () {
	var legend_label = "";
	// first fetch all relevant options
	options = new Array();

	options['xvar'] = getValue ("xvar");
	options['type'] = getValue ("type");
	if (options['type'] == "juxtaposed") {
		options['juxtaposed'] = true;
		options['labels'] = getValue ("labels");
		if (options['labels']) {
			options['place'] = getValue ("place");
		}
	} else {
		options['labels'] = false;
		options['juxtaposed'] = false;
	}
	options['legend'] = getValue ("legend");
	options['colors'] = getValue ("colors");

	// generate and print argument list suitable for display in rk.header
	if (options['legend']) legend_label = "TRUE";
	else legend_label = "FALSE";
	echo (', "colors"="' + options['colors'] + '", "Type"="' + options['type'] + '", "Legend"="' + legend_label + '"');
}


function printout () {
	var col_option = "";
	if (options['colors'] == 'rainbow') {
		col_option = ', col=rainbow (if(is.matrix(' + options['xvar'] + ')) dim(' + options['xvar'] + ') else length(' + options['xvar'] + '))';
	}

	// construct the main call to barplot
	var main_call = 'barplot(' + options['xvar'] + col_option;
	if (options['juxtaposed']) main_call += ', beside=TRUE';
	if (options['legend']) main_call += ', legend.text=TRUE';
	if (options['labels']) main_call += ", ylim = yrange";
	main_call += getValue ('plotoptions.code.printout');
	main_call += ")\n";

	var plot_pre = getValue ('plotoptions.code.preprocess');
	var plot_adds = getValue ('plotoptions.code.calculate');

	// now print everything as needed
	echo (plot_pre);

	if (options['labels']) {
		echo ('# adjust the range so that the labels will fit\n');
		echo ('yrange <- range (' + options['xvar'] + ', na.rm=TRUE) * 1.2\n');
		echo ('if (yrange[1] > 0) yrange[1] <- 0\n');
		echo ('if (yrange[2] < 0) yrange[2] <- 0\n');

		echo ("bplot <- ");
	}

	echo (main_call);

	if (options['labels']) {
		echo ('text (bplot,' + options['xvar'] + ', labels=' + options['xvar'] + ', pos=' + options['place'] + ', offset=.5)');
		echo ("\n");
	}

	echo (plot_adds);
}

