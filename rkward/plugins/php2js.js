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
				if (input.substr (i+2, 3) == "php") i += 3;
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
		else if (c == "\\") c = "\\\\";
//		else if (c == "\t") c = "\\t";
		else if (c == "\n") {
			c = "\\n');\n"
			in_echo = false;
		}

		first_char = false;
		output += c;

		if (i >= input.length) {
			message ("Something's wrong. Closure not found.");
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

		// comments
		if ((c == "/") && (cn == "/")) {
			var eol = input.indexOf ("\n", i);
			output += input.substring (i, eol);
			i = eol - 1;
			continue;
		}
		if (c == "#") {
			var eol = input.indexOf ("\n", i);
			output += "//" + input.substring (i + 1, eol);
			i = eol - 1;
			continue;
		}
		if ((c == "/") && (cn == "*")) {
			message ("Warning: multiline comments are not handled! Check by hand!");
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
			i += 8;
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
		} else if (input.indexOf ("array", i) == i) {
			output += "new Array";
			i += 5;
			continue;
		} else if (input.indexOf ("global ", i) == i) {
			i += 7 + eatGlobals (input.substr (i + 7));
			continue;
		} else if (input.indexOf ("function", i) == i) {
			current_fun = getFunctionName (input.substr (i+8));
			locals[current_fun] = new Array ();
			fun_args[current_fun] = new Array ();
			var args_start = input.indexOf ("(", i+8) + 1;
			var funb_level = 1;
			var args_end = args_start;
			while (funb_level) {
				if (input.charAt (args_end) == ")") {
					funb_level -= 1;
				} else if (input.charAt (args_end) == "(") {
					funb_level += 1;
				}
				args_end += 1;
			}
			eatFunctionArgs (input.substring (args_start, args_end - 1));
		}

		// associative array operator
		if ((c == "=") && (cn == ">")) {
			message ("Warning: please check correctness of conversion of '=>' in arrays by hand");
			output += ", ";
			i++;
			continue;
		}

		// string concatenation operator
		if (c == ".") c = "+";

		output += c;
	}

	message ("Something's wrong. Closing ?> not found.");
	return input.length;
}

function getFunctionName (input) {
	var open_paren = input.indexOf ("(");
	var fun_name = input.substring (0, open_paren - 1);
	fun_name = fun_name.replace (/\s*/g, "");
	return fun_name;
}

function convertPHPQuote (input, quote_char) {
	var inside_quote = "";
	var closed = false;

	for (var i = 0; i < input.length; ++i) {
		var c = input.charAt (i);

		// handle escapes first
		if (c == "\\") {
			inside_quote += c;
			inside_quote += input.charAt (++i);
			continue;
		}

		if ((c == "$") && (quote_char == "\"") && (pass == 1)) {
			if (input.charAt (i+1) != quote_char) {
				message ("Warning: '$' inside '\"'-delimited string. This might be a variable name. Please check by hand!");
			}
/*			token = getToken (input.substr (i + 1));
			output += quote_char + " + " + token;
			i += token.length;
			if (input.charAt (i + 1) != quote_char) output += " + " + quote_char;
			continue; */
		}

		// end of string
		if (c == quote_char) {
			closed = true;
			break;
		}

		inside_quote += c;
	}

/*	// unquote numeric constants
	if ((inside_quote.length > 0) && (!isNaN (inside_quote))) {
		output = output.substr (0, output.length -1);	// ugly hack: remove quote already added
		output += inside_quote;
	} else {
		output += inside_quote + quote_char;
	}*/
	output += inside_quote + quote_char;

	if (!closed) message ("Something's wrong. Closing " + quote_char + " not found.");
	return i + 1;
}

function getToken (input) {
	var i = input.search (/[^a-zA-Z0-9_]+/);
/*	if ((input.charAt (i) == "[") || (input.charAt (i+1) == "[")) {	// array subscripts
		i = input.indexOf ("]", i) + 1;
	}
	if (i < 1) {
		message ("Something's wrong. Token end not found. Token start was " + input.substr (0, 10));
		return (input);
	} */
	var token = input.substr (0, i);
/*	if (token.search (/\[\]/) != -1) {
		message ("Use of [] in token " + token + ". Please convert to 'X.push (Y)' by hand.");
	} */

	if (!contains(locals[current_fun], token)) locals[current_fun].push (token);
	return (token);
}

function eatGlobals (input) {
	var end = input.indexOf (";") + 1;
	var text = input.substr (0, end);
	var tokens = text.split (",");
	for (var i = 0; i < tokens.length; ++i) {
		var token = tokens[i].replace (/^[\$ ]*/, "");
		token = getToken (token + " ");
		if (!contains (globals, token)) globals.push (token);
	}
	return (end);
}

function eatFunctionArgs (input) {
	var tokens = input.split (",");
	for (var i = 0; i < tokens.length; ++i) {
		var token = tokens[i].replace (/^[\$ ]*/, "");
		token = getToken (token + " ");
		fun_args[current_fun].push (token);
	}
}

// this function is meant to be run in step 2 (i.e. on already js code)
function mergeEchos (line) {
	var output_save = output;
	output = "";

	var directly_after_echo = false;
	for (var i = 0; i < line.length; ++i) {
		var c = line.charAt (i);
		var cn = line.charAt (i + 1);

		// comments
		if ((c == "/") && (cn == "/")) {
			if (directly_after_echo) {
				output += "); ";
				directly_after_echo = false;
			}
			output += line.substr (i);
			break;
		}

		if (!directly_after_echo) {
			if ((c == "'") || (c == "\"")) {
				// hack: skips js quotes, too
				output += c;
				i += convertPHPQuote (line.substr (i + 1), c);
				continue;
			}
		}

		if (line.indexOf ("echo", i) == i) {
			i += 4;
			var output_save_2 = output;
			output = "";
			var cb = line.charAt (i);
			while (cb != ";") {
				if ((cb == "'") || (cb == "\"")) {
					output += cb;
					i += convertPHPQuote (line.substr (i + 1), cb);
				} else {
					output += cb;
				}
				++i;
				cb = line.charAt (i);
			}

			var fragment = output;
			output = output_save_2;
			if (i >= line.length) {
				message ("Strange echo statement. Please check by hand.");
				continue;
			}
			fragment = fragment.replace (/^\s*\(\s*/, "");
			fragment = fragment.replace (/\s*\)\s*$/, "");

			if (!directly_after_echo) {
				output += "echo (";
			} else {
				output += " + ";
			}
			output += fragment;

			directly_after_echo = true;
		} else {
			if (directly_after_echo) {
				if (c != " ") {
					output += "); " + c;
					directly_after_echo = false;
				}
			} else {
				output += c;
			}
			
			if (line.indexOf ("if", i) == i) {
				output += "f";
				i += feedthroughControlStatement (line.substr (i+2)) + 2;
			}
		}
	}

	if (directly_after_echo) {
		output += ");";
	}

	var ret = output;
	output = output_save;
	return (ret);
}

function feedthroughControlStatement (input) {
	var levelstack = new Array ();
	for (var i = 0; i < input.length; ++i) {
		var c = input.charAt (i);

		output += c;

		if ((c == "{") || (c == "(")) {
			levelstack.push (c);
		} else if (c == "}") {
			if (levelstack[levelstack.length - 1] == "{") {
				levelstack.pop ();
				if (levelstack.length == 0) {
					return i;
				}
			} else {
				message ("Warning: Brace mismatch while postprocessing " + input);
			}
		} else if (c == ")") {
			if (levelstack[levelstack.length - 1] == "(") {
				levelstack.pop ();
			} else {
				message ("Warning: Brace mismatch while postprocessing " + input);
			}
		} else if (c == ";") {
			if (levelstack.length == 0) {
				message ("Note: Control statement without braces. This is bad style. ");
				return i;
			}
		}
	}
	return i;	// end of line reached is an ok condition
}

function contains (array, token) {
	for (var i = 0; i < array.length; ++i) {
		if (array[i] == token) return (true);
	}
	return (false);
}

function postProcess (input) {
	var lines = input.split ("\n");
	var olines = new Array ();

	for (var i = 0; i < lines.length; ++i) {
		if (lines[i].search (/^function /) >= 0) {
			var fun_name = getFunctionName (lines[i].substr (8));

			olines.push (lines[i]);
			while (lines[++i].search (/^\s*$/) >= 0) {
				// skip empty line
			}
			if (lines[i] == "}") {
				// kill entire function
				olines.pop ();
				continue;
			}

			for (var l = 0; l < locals[fun_name].length; ++l) {
				if (!contains (globals, locals[fun_name][l])) {
					if (!contains (fun_args[fun_name], locals[fun_name][l])) {
						olines.push ('var ' + locals[fun_name][l] + ' = "";');
					}
				}
			}
		}
		// fix includes
		lines[i] = lines[i].replace (/^include\s*\(\s*[\"\']([^\)]*)\.php[\"\']\s*\)/, "include ('$1.js')");

//		olines.push (lines[i]);
		olines.push (mergeEchos (lines[i]));
	}

	return (olines.join ("\n"));
}

function message (text) {
	_message += text + "\n";
	print (text);
}

filename = arguments[0];
file = readFile (filename);
print ("--------- converting file " + filename);

var _message = "";
// the output buffer
var output = "";
// list of global vars
var globals = new Array ();
var locals = new Object ();
var fun_args = new Object ();
fun_args.none = new Array ();
locals.none = new Array ();
var current_fun = "none";
var pass = 1;

// main conversion step
convertTopLevel (file);
pass = 2;
output = postProcess (output);

// add global var declarations
for (var i = globals.length; i >= 0; --i) {
	output = "var " + globals[i] + ";\n" + output;
	if (i == 0) output = "// globals\n" + output;
}
output = "/* ------- This file generated by php2js from PHP code. --------\nPlease check this file by hand, and remove this notice, afterwards.\nMessages:\n" + _message + "\n---------------------------- */\n\n" + output;

// write to file
importPackage(java.io); // From rhino directory
function writeFile(file, content) {
	var buffer = new PrintWriter( new FileWriter(file) );
	buffer.print(content);
	buffer.flush();
	buffer.close();
} 
outfile = arguments[0].replace (/\.php$/, ".js");
writeFile (outfile, output);
