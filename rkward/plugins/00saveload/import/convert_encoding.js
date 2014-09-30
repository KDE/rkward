function makeEncodingPreprocessCode () {
	if (!getValue ("do_locale_conversion")) return;
	echo ('\n');
	echo ('# helper function to convert all strings to the current encoding\n');
	echo ('iconv.recursive <- function (x, from) {\n');
	echo ('	attribs <- attributes (x);\n');
	echo ('	if (is.character (x)) {\n');
	echo ('		x <- iconv (x, from=from, to="", sub="")\n');
	echo ('	} else if (is.list (x)) {\n');
	echo ('		x <- lapply (x, function (sub) iconv.recursive (sub, from))\n');
	echo ('	}\n');
	echo ('	# convert factor levels and all other attributes\n');
	echo ('	attributes (x) <- lapply (attribs, function (sub) iconv.recursive (sub, from))\n');
	echo ('	x\n');
	echo ('}\n');
}

function makeEncodingCall (varname) {
	if (!getValue ("do_locale_conversion")) return;

	var from_locale = getValue ("encoding");
	if (from_locale == "other") {
		from_locale = getValue ("user_encoding");
	}
	echo ('\n');
	echo ('# convert all strings to the current encoding\n');
	echo (varname + ' <- iconv.recursive (' + varname + ', from="' + from_locale + '")\n');
}