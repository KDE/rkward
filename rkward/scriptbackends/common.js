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

printIndentedUnlessEmpty = function (indentation, lines, pre, post) {
	if (lines.length <= 0) return;
	if (typeof (pre) != 'undefined') echo (pre);
	printIndented (indentation, lines);
	if (typeof (post) != 'undefined') echo (post);
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

getString = function (id) {
	// getValue() sometimes returns numeric results (whenever the value is "0"). This variant always returns strings.
	return (_RK_backend.getString (id));
}

getList = function (id) {
	// will try to return value as list, and produces a warning, if that fails.
	return (_RK_backend.getList (id));
}

getBoolean = function (id) {
	// will try to return value as logical, and produces a warning, if that fails.
	return (_RK_backend.getBoolean (id));
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
