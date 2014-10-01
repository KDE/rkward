var input;
var output;

function calculate () {
	input = getString ("x");
	output = input;
	if (getBoolean ("saveto_other_object")) output = getString ("saveto");
	var datamode = getString ("datamode");
	var is_factor = datamode == "factor";
	var is_character = datamode == "character";
	var default_values = getString ("other");

	// initialize output vector to defaults
	echo ('input <- ' + input + '\n');
	if (is_factor) {
		echo ('# Use as.character() as intermediate data format, to support adding and dropping levels\n');
		echo ('recoded <- as.character (');
	} else {
		echo ('recoded <- as.' + datamode + ' (');
	}
	if (default_values == "copy") {
		echo (input + ")\n");
	} else {
		echo ('rep (' + default_values == "na" ? 'NA' : getString ("other_custom.valuequoted") + ', length.out = length (' + input + '))\n');
	}

	// Make replacements
	var old_value_types = getList ("set.old_value_types");
	var old_value_strings = getList ("set.old_value_strings");
	var new_value_types = getList ("set.new_value_types");
	var new_value_strings = getList ("set.new_value_strings");

	var dupes = Array ();
	var all_old_values = Array ();
	for (var i = 0; i < old_value_types.length; ++i) {
		var old_values_it;
		if (old_value_types[i] == "na") old_values_it = Array ("NA");
		else old_values_it = old_value_strings[i].split ('\n');

		// Check for duplicate old values, as the UI does not currently support this!
		for (var j = 0; j < old_values_it.length; ++j) {
			if (all_old_values.indexOf (old_values_it[j]) > -1) {
				dupes.push (old_values_it[j]);
			} else {
				all_old_values.push (old_values_it[j]);
			}
		}

		var old_index;
		if (old_values_it.length > 1) {
			old_index = 'input %in% c(' + old_values_it.join (',') + ')';
		} else {
			if (old_values_it == "NA") old_index = 'is.na (input)';
			else old_index = 'input == ' + old_values_it;
		}

		var replacement = 'NA';
		if (new_value_types[i] != "na") {
			replacement = new_value_strings[i];
			if (is_factor || is_character) replacement = quote (replacement);
			else if (datamode == "logical") replacement = Number (replacement) ? "TRUE" : "FALSE";
		}

		echo ('recoded[' + old_index + '] <- ' + replacement + '\n');
	}

	// Produce warnings (or should it be errors?)
	if (dupes.length > 0) {
		echo ('\nwarning ("Some input values were specified more than once: ", ' + quote (dupes.join (', ')) + ')\n');
	}

	// Set data type and assign to output variable in GlobalEnv
	echo ('.GlobalEnv$' + output + ' <- ');
	if (is_factor) echo ('as.factor (recoded)\n');
	else echo ('recoded\n');
}

function printout () {
// TODO: Number of differences always shows as 0, if storing to same object...
	makeHeaderCode ('Recode categorical data', new Array ('Input variable', input, 'Output variable', output, 'Number of differences after recoding', noquote ('sum (' + input + ' != ' + output + ', na.rm=TRUE) + sum (is.na (' + input + ') != is.na (' + output + '))')));
}
