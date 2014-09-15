function calculate () {
	var input = getString ("x");
	var output = input;
	if (getBoolean ("saveto_other_object")) output = getString ("saveto");
	var datamode = getString ("datamode");
	var quote_new_values = (datamode == "character" || datamode == "factor");

	// initialize output vector to defaults
	// TODO

	// Make replacements
	var old_values = getList ("set.values");
	var new_values_types = getList ("set.new_value_types");
	var new_values_strings = getList ("set.new_value_strings");

	var dupes = Array ();
	var all_old_values = Array ();
	for (var i = 0; i < old_values.length; ++i) {
		var old_values_it = old_values[i].split ('\n');

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
			old_index = ' %in% c(' + old_values_it.join (',') + ')';
		} else {
			old_index = ' == ' + all_old_values;
		}

		var replacement = "NA";
		if (new_values_types[i] != "na") {
			replacement = new_values_strings[i];
			if (quote_new_values) replacement = quote (replacement);
		}

		echo (output + '[' + input + old_index + '] <- ' + replacement + '\n');
	}

	// Produce warnings (or errors?) TODO
	if (dupes.length > 0) {
		echo ("Dupes: " + dupes.join (", "));
	}

	// Drop NAs TODO
	// Set data type TODO
	// Assign to output variable in GlobalEnv TODO
}
