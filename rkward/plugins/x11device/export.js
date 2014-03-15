function preprocess () {
	if (getValue ("format") == "svg") {
		echo ('if (!capabilities ("cairo")) {\n');
		echo ('	# The "cairo" library, providing SVG-support, is not compiled in by default on some systems.\n');
		echo ('	require (cairoDevice)\n');
		echo ('	svg <- Cairo_svg\n');
		echo ('}\n');
	} else if (getValue ("format") == "tikz") {
		echo ('require (tikzDevice)\n');
	}
}

function calculate () {
	var type = getValue ("format");
	var file = getValue ("file");

	var gstype = "";
	var jpegpng = "";
	var eps = "";
	var ext = "";
	if (type == "gs") {
		gstype = getValue ("gsformat");
		if (gstype == "other") gstype = getValue ("gs_specifiedformat");
	} else {
		jpegpng = ((type == "jpeg") | (type == "png"));
		eps = (type == "postscript") && (getValue ("formateps"));

		// Does the filename end with .ps/.eps or .pdf or .png or .jpeg/.jpg or .tex?
		// If not, add the appropriate extension.
		if (getValue ("autoextension")) {
			if (type == "jpeg") {
				if (file.search (/\.jpg$/i) < 0) file += ".jpg";
			} else if (type == "postscript") {
				if (eps) {
					if (file.search (/\.e?ps$/i) < 0) file += ".eps";
				} else {
					if (file.search (/\.e?ps$/i) < 0) file += ".ps";
				}
			} else if (type == "tikz") {
				if (file.search (/\.tex$/i) < 0) file += ".tex";
			} else {
				var regexp = new RegExp ("\." + type + "$", "i");
				if (file.search (regexp) < 0) file += "." + type;
			}
		}
	}
	var options = "";
	if ((type == "postscript") && eps) options += ", onefile=FALSE";

	var autores = "";
	var resolution = "";
	// set $resolution appropriately:
	if (jpegpng || (type == "gs")) {
		autores = getValue ("autores");
		if (autores) {
			if (jpegpng) resolution = 96;
			else resolution = 72;
		}	else resolution = getValue ("resolution");
	}

	var autoW = getValue ("autowidth");
	var autoH = getValue ("autoheight");

	// jpeg()/png() need at least one of width/height. For jpeg()/png() the width/height parameter (in pixels)
	// is calculated using width/height (in inches) times the resolution. For postscript()/pdf() $resolution is set to 1.
	if (jpegpng && autoW && autoH) {
		options += ", width=par(\"din\")[1]*" + resolution;
	} else if (jpegpng) {
		if (!autoW) options += ", width=" + Math.round(getValue ("width")*resolution);
		if (!autoH) options += ", height=" + Math.round(getValue ("height")*resolution);
	} else {
		if (!autoW) options += ", width=" + getValue ("width");
		if (!autoH) options += ", height=" + getValue ("height");
	}

	// pointsize, resolution and quality parameters:
	if (!getValue ("autopointsize")) options += ", pointsize=" + getValue ("pointsize");
	if ((jpegpng && !autores) || (type == "gs")) options += ", res=" + resolution;
	if ((type == "jpeg") && (!getValue ("autoquality"))) options += ", quality=" + getValue ("quality");

	// For ps/pdf: page, pagecentre, horizontal, family, encoding and title parameters:
	if (!jpegpng) {
		var paper = "";
		if (!(eps)) paper = getValue ("paper");
		else paper = "special";
		if (paper != "") options += ", paper=" + "\"" + paper + "\"";

		var pagecentre = getValue ("pagecentre");
		if (!pagecentre) options += ", pagecentre=FALSE";

		var pshoriz = "";
		if (!eps) pshoriz = getValue ("ps_horiz");
		else pshoriz = false;
		if (!pshoriz) options += ", horizontal=FALSE";

		var family = getValue ("family");
		if (family != "") options += ", family=" + "\"" + family + "\"";

		var enc = getValue ("encoding");
		if (enc != "") options += ", encoding=" + "\"" + enc + "\"";

		if (!getValue("autotitle")) options += ", title=" + "\"" + getValue("title") + "\"";
	}

	echo ('dev.set (' + getValue ("devnum") + ')\n');
	if (type == "gs") {
		echo ('dev2bitmap ("' + file + '", type="' + gstype + '"' + options + ');\n');
	} else {
		echo ('dev.print (device=' + type + ', file="' + file + '"' + options + ');\n');
	}
}

