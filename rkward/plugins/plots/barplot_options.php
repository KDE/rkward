<?
function preprocess () {
	// first fetch all relevant options
	global $options;
	$options = array ();

	$options['xvar'] = getRK_val ("xvar");
	$options['type'] = getRK_val ("type");
	if ($options['type'] == "juxtaposed") {
		$options['juxtaposed'] = true;
		$options['labels'] = getRK_val ("labels");
		if ($options['labels']) {
			$options['place'] = getRK_val ("place");
		}
	} else {
		$options['labels'] = false;
		$options['juxtaposed'] = false;
	}
	$options['legend'] = getRK_val ("legend");
	$options['colors'] = getRK_val ("colors");

	// generate and print argument list suitable for display in rk.header
	if ($options['legend']) $legend_label = "TRUE";
	else $legend_label = "FALSE";
	echo (', "colors", "' . $options['colors'] . '", "Type", "' . $options['type'] . '", "Legend", "' . $legend_label . '"');
}

function calculate () {
}

function printout () {
	global $options;

	if ($options['colors'] == 'rainbow') {
		$col_option = ', col=rainbow (if(is.matrix(' . $options['xvar'] . ')) dim(' . $options['xvar'] . ') else length(' . $options['xvar'] . '))';
	}

	// construct the main call to barplot
	$main_call = 'barplot(' . $options['xvar'] . $col_option;
	if ($options['juxtaposed']) $main_call .= ', beside=TRUE';
	if ($options['legend']) $main_call .= ', legend.text=TRUE';
	if ($options['labels']) $main_call .= ", ylim = yrange";
	$main_call .= ")\n";

	// now print everything as needed
	if ($options['labels']) { ?>
# adjust the range so that the labels will fit
yrange <- range (<? echo ($options['xvar']); ?>, na.rm=TRUE) * 1.2
if (yrange[1] > 0) yrange[1] <- 0
if (yrange[2] < 0) yrange[2] <- 0
<?	
		echo ("bplot <- ");
	}

	echo ($main_call);

	if ($options['labels']) {
		echo ('text (bplot,' . $options['xvar'] . ', labels=' . $options['xvar'] . ', pos=' . $options['place'] . ', offset=.5)');
		echo ("\n");
	}
}
?>
