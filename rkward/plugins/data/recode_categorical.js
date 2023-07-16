/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
var input;
var output;

function preview () {
	calculate (true);
}

function calculate (is_preview) {
	input = getString ("x");
	output = input;
	if (getBoolean ("saveto_other_object")) output = getString ("saveto");
	var datamode = getString ("datamode");
	var is_factor = datamode == "factor";
	var is_character = datamode == "character";
	var default_values = getString ("other.string");

	// initialize output vector to defaults
	echo ('input <- ' + input + '\n');
	if (is_factor) {
		comment ('Use as.character() as intermediate data format, to support adding and dropping levels');
		echo ('recoded <- as.character (');
	} else {
		echo ('recoded <- as.' + datamode + ' (');
	}
	if (default_values == "copy") {
		echo (input + ")\n");
	} else {
		echo ('rep (' + ((default_values == "na") ? 'NA' : getString ("other_custom.valuequoted")) + ', length.out = length (' + input + ')))\n');
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
		echo ('\nwarning (' + i18n ("Some input values were specified more than once: ") + ', ' + quote (dupes.join (', ')) + ')\n');
	}

	// Set data type and assign to output variable in GlobalEnv / preview
	var result = is_factor ? 'as.factor (recoded)' : 'recoded';
	if (is_preview) {
		echo ('temp <- data.frame ("Old values"=I(' + input + '), "New values"=I(' + result + '))\n');
		echo ('preview_data <- temp[1:min(dim(temp)[1],500),]\n');
	} else {
		echo ('.GlobalEnv$' + output + ' <- ' + result + '\n');
	}
}

function printout () {
// TODO: Number of differences always shows as 0, if storing to same object...
	new Header (i18n ("Recode categorical data"))
		.add (i18n ("Input variable"), input)
		.add (i18n ("Output variable"), output)
		.add (i18n ("Number of differences after recoding"), noquote ('sum (' + input + ' != ' + output + ', na.rm=TRUE) + sum (is.na (' + input + ') != is.na (' + output + '))'))
		.print ();
}
