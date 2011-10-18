_script_output = "";
/* NOTE: for compatibility with Kross, we can't write this as 'function echo (text)...', somehow. */
echo = function (text) {
	_script_output += text;
}

printIndented = function (indentation, lines) {
	/** More complex than may seem necessary at first glance. The point is that the
	final line break in the input should *not* be replaced by line break + indentation. */
	echo (indentation + lines.replace (/\n(.)/g, "\n" + indentation + "$1"));
}

noquote = function (text) {
	var ret = new String (text);
	ret.noquote = 1;
	return (ret);
}

quote = function (text) {
	if (text.noquote) return text;
	return ("\"" + text.replace (/\"/g, "\\\"") + "\"");
}

makeHeaderCode = function (title, parameters) {
	echo ("rk.header(" + quote (title));
	if (parameters.length) {
		echo (", parameters=list(");
		for (var p = 0; p < parameters.length; ++p) {
			if (p) {
				if (!(p % 2)) echo (",\n\t");
				else echo (", ");
			}
			echo (quote(parameters[p]));
		}
		echo (")");
	}
	echo (")\n");
}

getValue = function (id) {
	return (_RK_backend.getValue (id));
}

printValue = function (id) {
	echo (getValue (id));
}

include = function (file) {
	_RK_backend.includeFile (file);
}

flushOutput = function () {
	var string = _script_output;
	_script_output = "";
	return (string);
}

do_preprocess = function () {
	if (typeof (preprocess) == "undefined") return ("");
	preprocess ();
	return (flushOutput ());
}

do_calculate = function () {
	if (typeof (calculate) == "undefined") return ("");
	calculate ();
	return (flushOutput ());
}

do_printout = function () {
	if (typeof (printout) == "undefined") return ("");
	printout ();
	return (flushOutput ());
}

do_preview = function () {
	if (typeof (preview) == "undefined") return ("");
	preview ();
	return (flushOutput ());
}

// for compatibility with the converted PHP code
trim = function (text) {
	var ret = text.replace (/^\s*/, "").replace (/\s*$/, "");
	return (ret);
}

split = function (by, text) {
	return (text.split (by));
}

function str_replace (needle, replacement, haystack) {
	return (haystack.split (needle).join (replacement));
}

// should behave similar to rk.get.short.name()
shortName = function (variables) {
	var snArray = variables.split("\n");
	var snArrayShort = new Array();
	for(var i=0, len=snArray.length; value=snArray[i], i<len; i++) {
		var loopTmp = value.slice(value.lastIndexOf("[[\"")+3, value.lastIndexOf("\"]]"));
		if(loopTmp == "") {
			snArrayShort.push(value);
		} else {
			snArrayShort.push(loopTmp);
		}
	}
	return(snArrayShort);
}

// shortcut to get shortnames from varslots directly, as a
// replacement for getValue()
// append '.join(", ")' to get then in a comma separated string
getValueShort = function (variable) {
	var gvsVar = getValue(variable);
	gvsVar = shortName(gvsVar);
	return(gvsVar);
}
