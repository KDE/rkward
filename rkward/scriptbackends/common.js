_script_output = "";
function echo (text) {
	_script_output += text;
}

function printIndented (indentation, lines) {
	echo (indentation + lines.replace (/\n/g, "\n" + indentation));
}

function noquote (text) {
	ret = new String (text);
	ret.noquote = 1;
	return (ret);
}

function quote (text) {
	if (text.noquote) return text;
	return ("\"" + text.replace (/\"/g, "\\\"") + "\"");
}

function makeHeaderCode (title, parameters) {
	echo ("rk.header(" + quote (title));
	if (parameters.length) {
		echo (", parameters=list(");
		for (var p = 0; p < parameters.length; ++p) {
			if (p) {
				echo (", ");
				if (!(p % 2)) echo ("\n\t");
			}
			echo (quote(parameters[p]));
		}
		echo (")");
	}
	echo (")\n");
}

function getValue (id) {
	return (_RK_backend.getValue (id));
}

function printValue (id) {
	echo (getValue (id));
}

function include (file) {
	_RK_backend.includeFile (file);
}

function flushOutput () {
	var string = _script_output;
	_script_output = "";
	return (string);
}

function do_preprocess () {
	if (typeof (preprocess) == "undefined") return ("");
	preprocess ();
	return (flushOutput ());
}

function do_calculate () {
	if (typeof (calculate) == "undefined") return ("");
	calculate ();
	return (flushOutput ());
}

function do_printout () {
	if (typeof (printout) == "undefined") return ("");
	printout ();
	return (flushOutput ());
}

function do_preview () {
	if (typeof (preview) == "undefined") return ("");
	preview ();
	return (flushOutput ());
}

// for compatibility with the converted PHP code
function trim (text) {
	var ret = text.replace (/^\s*/, "").replace (/\s*$/, "");
	return (ret);
}

function split (by, text) {
	return (text.split (by));
}

function str_replace (needle, replacement, haystack) {
	return (haystack.split (needle).join (replacement));
}
