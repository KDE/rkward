function calculate () {
	var titlevar = getValue ("titlevar");
	var outvar = getValue ("outvar");
	var groups = getValue ("groups").split ("\n").join (", ");
	var stat = getValue ("stat");
	var fun = "length";
	echo ('groups <- rk.list (' + groups + ')\n');
	if (titlevar != "") echo (titlevar + ' <- paste (names (groups), collapse=" by ")\n');

	if (stat == "freq") {
		echo (outvar + ' <- table (interaction (groups))\n');
	} else {
		if (stat == "sum") {
			fun = "sum";
		} else {
			fun = 'function (x) { ' + getValue ("custom_stat") + ' }';
		}
		echo (outvar + ' <- by (' + getValue ("outcome") +', interaction (groups), FUN=' + fun + ')\n');
	}
}

