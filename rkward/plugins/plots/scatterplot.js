/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	var x = getList ("x").join (",");
	var y = getList ("y").join (",");

	var type = "";
	if (getValue ("manual_type.numeric")) {
		type = getValue ("custom_type");
	} else {
		type = "c ('" + getValue ("pointtype") + "')";
	}
	var col = getValue ("col");
	var pch = getValue ("pch");
	var cex = getValue ("cex");

// input
	echo ('Xvars <- rk.list(' + x + ')\n');
	echo ('Yvars <- rk.list(' + y + ')\n');
	echo ('\n');

// verification (is this needed?) ?>
	echo ('if (length(Xvars) != length(Yvars)) {\n');
	echo ('	stop(' + i18n ("Unequal number of X and Y variables given") + ')\n');
	echo ('}\n');

	comment ('find range of X/Y values needed');
	echo ('Xrange <- range (c (Xvars), na.rm=TRUE)\n');
	echo ('Yrange <- range (c (Yvars), na.rm=TRUE)\n');
	echo ('\n');
	echo ('type <- rep (' + type + ', length.out=length (Xvars));\n');
	echo ('col <- rep (' + col + ', length.out=length (Xvars));\n');
	echo ('cex <- rep (' + cex + ', length.out=length (Xvars));\n');
	echo ('pch <- rep (' + pch + ', length.out=length (Xvars));\n');
}

function printout () {
	doPrintout (true);
}

function preview () {
	calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	if (full) {
		new Header (i18n ("Scatterplot"))
			.add (i18n ("X variables"), noquote ('paste (names (Xvars), collapse=", ")'))
			.add (i18n ("Y variables"), noquote ('paste (names (Yvars), collapse=", ")')).print ();
		echo ('\n');
		echo ('rk.graph.on()\n');
		echo ('\n');
	}
	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	# make frame and axes\n');
	echo ('	plot(Xrange, Yrange, type="n"' + getValue ("plotoptions.code.printout") + ')\n');
	echo ('	\n');
	echo ('	# plot variables one X/Y pair at a time\n');
	echo ('	for (i in 1:length(Xvars)) {\n');
	echo ('		points (\n');
	echo ('			Xvars[[i]],\n');
	echo ('			Yvars[[i]],\n');
	echo ('			type = type[[i]],\n');
	echo ('			col = col[[i]],\n');
	echo ('			cex = cex[[i]],\n');
	echo ('			pch = pch[[i]]\n');
	echo ('		)\n');
	echo ('	}\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('\n');
		echo ('rk.graph.off()\n');
	}
}

