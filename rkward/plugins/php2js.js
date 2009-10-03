// php2js.js a small converter for PHP-code -> JS-code.
//
// usage: rhino php2js.js file.php > file.js
//
// Anybody stumbling on this, please note that this covers only a very limited range of functionality,
// specific to the needs of the RKWard project. Give it a try if you like, but probably it does not
// help you much.

function convertTopLevel (input) {
	var in_echo = false;
	var first_char = true;
	for (var i = 0; i < input.length; ++i) {
		var c = input.charAt (i);
		var cn = input.charAt (i+1);

		// start of PHP block detection
		if (c == "<") {
			if (cn == "?") {
				if (in_echo) {
					output += "'); ";
					in_echo = false;
				}
				i += 1 + convertPHPBlock (input.substr (i+2));
				first_char = true;
				continue;
			}
		}

		// fallthrough: echo it
		if (!in_echo) {
			if ((first_char) && (c == "\n")) {
				output += "\n";
				first_char = false;
				continue;
			}

			output += "echo ('";
			in_echo = true;
		}

		if (c == "'") c = "\\'";
//		else if (c == "\t") c = "\\t";
		else if (c == "\n") {
			c = "\\n');\n"
			in_echo = false;
		}

		first_char = false;
		output += c;

		if (i >= input.length) {
			print ("Something's wrong. Closure not found.");
		}
	}
}

function convertPHPBlock (input) {
	for (var i = 0; i < input.length; ++i) {
		var c = input.charAt (i);
		var cn = input.charAt (i+1);

		if (i == 0) {
			if (c == " ") continue;
			if ((c == "\n") && (output[output.length - 1] == "\n")) continue;
		}

		// end of PHP block detection
		if (c == "?") {
			if (cn == ">") {
				return (i + 2);
			}
		}

		// handle quotes
		if (c == "\"") {
			output += c;
			i += convertPHPQuote (input.substr (i+1), "\"");
			continue;
		}
		if (c == "\'") {
			output += c;
			i += convertPHPQuote (input.substr (i+1), "\'");
			continue;
		}

		if (c == "$") {
			var token = getToken (input.substr (i + 1));
			output += token;
			i += token.length;
			continue;
		}

		// replace some functions
		if (input.indexOf ("getRK_val", i) == i) {
			output += "getValue";
			i += 9;
			continue;
		} else if (input.indexOf ("getRK", i) == i) {
			// replace with an echo form to allow later merging of echo statements
			output += "echo (getValue";
			closure = input.indexOf (")", i);
			output += input.substring (i + 5, closure + 1) + ")";
			i = closure;
			continue;
		} else if (input.indexOf ("Array", i) == i) {
			output += "new ";
		} else if (input.indexOf ("global ", i) == i) {
			i += 7 + eatGlobals (input.substr (i + 7));
			continue;
		}

		// associative array operator
		if ((c == "=") && (cn == ">")) {
			print ("Warning: please check correctness of conversion of arrays by hand");
			output += ", ";
			i++;
			continue;
		}

		// string concatenation operator
		if (c == ".") c = "+";

		output += c;
	}

	print ("Something's wrong. Closing ?> not found.");
	return input.length;
}

function convertPHPQuote (input, quote_char) {
	for (var i = 0; i < input.length; ++i) {
		var c = input.charAt (i);

		// handle escapes first
		if (c == "\\") {
			output += c;
			output += input.charAt (++i);
			continue;
		}

		if ((c == "$") && (quote_char == "\"")) {
			token = getToken (input.substr (i + 1));
			output += quote_char + " + " + token;
			i += token.length;
			if (input.charAt (i + 1) != quote_char) output += " + " + quote_char;
			continue;
		}

		// end of string
		if (c == quote_char) {
			output += c;
			return (i + 1);
		}

		output += c;
	}

	print ("Something's wrong. Closing " + quote_char + " not found.");
	return input.length;
}

function getToken (input) {
	var i = input.search (/[^a-zA-Z0-9_]+/);
	if ((input.charAt (i) == "[") || (input.charAt (i+1) == "[")) {	// array subscripts
		i = input.indexOf ("]", i);
	}
	if (i < 1) {
		print ("Something's wrong. Token end not found. Token start was " + input.substr (0, 10));
		return (input);
	}
	return (input.substr (0, i));
}

function eatGlobals (input) {
	var end = input.indexOf (";") + 1;
	var text = input.substr (0, end);
	var tokens = text.split (",");
	for (var i = 0; i < tokens.length; ++i) {
		globals.push (getToken (tokens[i].substr (1)));
	}
	return (end);
}






// the output buffer
var output = "";
// list of global vars
globals = new Array ();

file = readFile (arguments[0]);

// main conversion step
convertTopLevel (file);

// add global var declarations
globals.sort ();
var prev_token;
for (var i = globals.length; i >= 0; --i) {
	if (prev_token != globals[i]) {		// print each var only once
		prev_token = globals[i];
		output = "var " + globals[i] + ";\n" + output;
	}
	if (i == 0) output = "// globals\n" + output;
}
print (output);
