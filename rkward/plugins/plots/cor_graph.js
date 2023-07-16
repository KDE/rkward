/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('cor.graph <- function(x) {\n');
	echo ('	panel.cor <- function(x, y, digits=' + getValue ("digits") + ', cex.cor, use="' + getValue ("use") + '", method="' + getValue ("method") + '", scale=' + getValue ("scale") + ') {\n');
	echo ('		usr <- par("usr"); on.exit(par(usr=usr))\n');
	echo ('		par(usr = c(0, 1, 0, 1))\n');
	echo ('		r <- abs(cor(x, y, use=use, method=method))\n');
	echo ('		txt <- format(c(r, 0.123456789), digits=digits)[1]\n');
	echo ('		if(missing(cex.cor)) cex <- 0.8/strwidth(txt)\n');
	echo ('	\n');
	echo ('		test <- cor.test(x,y, use=use, method=method)\n');
	echo ('		Signif <- symnum(test$p.value, corr = FALSE, na = FALSE,\n');
	echo ('				cutpoints = c(0, 0.001, 0.01, 0.05, 0.1, 1),\n');
	echo ('				symbols = c("***", "**", "*", ".", " "))\n');
	echo ('\n');
	echo ('		if(scale) text(0.5, 0.5, txt, cex = cex * r)\n');
	echo ('		else text(0.5, 0.5, txt, cex = cex)\n');
	echo ('		text(.8, .8, Signif, cex=cex, col=2)\n');
	echo ('	}\n');
	echo ('\n');
	echo ('	pairs(x, lower.panel=panel.smooth, upper.panel=panel.cor)\n');
	echo ('}\n');
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var vars = trim (getValue ("x")).replace (/\n/g, ",");

	echo ('data <- data.frame (' + vars + ')\n');
	echo ('\n');

	if (full) {
		new Header (i18n ("Correlation Matrix Plot")).addFromUI ("method").addFromUI ("use").addFromUI ("digits").addFromUI ("scale").print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try ({\n');
	echo ('	cor.graph (data)\n');
	echo ('})\n');

	if (full) {
		echo ('rk.graph.off ()\n');
		echo ('\n');
		echo ('rk.print('+ quote (i18n (noquote ("Legend:")) + '\\t\'***\': p &lt; 0.001 -- \'**\': p &lt; 0.01 -- \'*\': p &lt; 0.05 -- \'.\'\': p &lt; 0.1') + ')\n');
	}
}

