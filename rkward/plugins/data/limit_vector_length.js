function calculate () {
	var invar = getValue ("invar");
	var outvar = getValue ("outvar");

	echo ('max.categories <- ' + getValue ("cutoff") + '\n');
	echo ('if (length (' + invar + ') > max.categories) {\n');
	echo ('\t' + invar + ' <- sort (' + invar);
	if (getValue ("sorting") != "lowest") echo (', decreasing=TRUE');
	echo (')\n');
	if (getValue ("include_others.checked")) {
		var others_labelling = getValue ("others_label");
		if (others_labelling != "") others_labelling = quote (others_labelling) + '=';
		echo ('\tothers <- ' + invar + '[(max.categories+1):length(' + invar + ')]\n');
		
		var fun = getValue ("others_statistic");
		if (fun == "custom") {
			echo ('\tfun <- function (x) ' + getValue ("custom_stat") + '\n');
			fun = "fun";
		}

		echo ('\t' + outvar + ' <- c (' + invar + '[1:max.categories], ' + others_labelling + fun + ' (others)' + ')\n');
	} else {
		echo ('\t' + outvar + ' <- ' + invar + '[1:max.categories]\n');
	}
	echo ('}\n');
}

