/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var use;
var method;
var polyCorr;
var do_p;
var toNumeric;

function preprocess() {
	method = "\"" + getValue ("method") + "\"";
	if (method == "\"polyserial\"" || method == "\"polychoric\""){
		polyCorr = true;
	} else {
		polyCorr = false;
	}
	if (polyCorr) {
		echo ('require(polycor)\n');
	} else {}
}

function calculate () {
	do_p = getValue ("do_p");

	var exclude_whole = "";
	var vars = trim (getValue ("x"));
	toNumeric = getValue ("to_numeric");
	use = getValue ("use");
	if (use == "pairwise") {
		exclude_whole = false;
		if (!polyCorr) {
			use = "\"pairwise.complete.obs\"";
		} else {
			use = "\"pairwise\"";
		}
	} else {
		exclude_whole = true;
		if (!polyCorr) {
			use = "\"complete.obs\"";
		} else {
			use = "\"complete cases\"";
		}
	}

	comment ('cor requires all objects to be inside the same data.frame.');
	comment ('Here we construct such a temporary frame from the input variables');
	echo ('data.list <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	if (!polyCorr && toNumeric) {
		comment ('Non-numeric variables will be treated as ordered data and transformed into numeric ranks');
		echo ('transformed.vars <- list()\n');
		echo ('for (i in names(data.list)) {\n');
		echo ('	if(!is.numeric(data.list[[i]])){\n');
		echo ('		before.vars <- as.character(unique(data.list[[i]]))\n');
		echo ('		data.list[[i]] <- xtfrm(data.list[[i]])\n');
		echo ('		after.vars <- unique(data.list[[i]])\n');
		echo ('		names(after.vars) <- before.vars\n');
		comment ('Keep track of all transformations', '		');
		echo ('		transformed.vars[[i]] <- data.frame(rank=sort(after.vars))\n');
		echo ('	} else {}\n');
		echo ('}\n');
		comment ('Finally combine the actual data');
	} else {}
	echo ('data <- as.data.frame (data.list, check.names=FALSE)\n');
	echo ('\n');
	comment ('calculate correlation matrix');
	if (polyCorr) {
		echo ('result <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))\n');
	} else {
		echo ('result <- cor (data, use=' + use + ', method=' + method + ')\n');
	}
	if (do_p || polyCorr) {
		comment ('calculate matrix of probabilities');
		echo ('result.p <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))\n');
		if (exclude_whole) {
			comment ('as we need to do pairwise comparisons for technical reasons,\nwe need to exclude incomplete cases first to match the use="complete.obs" parameter in cor()');
			echo ('data <- data[complete.cases (data),]\n');
		} else {}
		echo ('for (i in 1:length (data)) {\n');
		echo ('	for (j in i:length (data)) {\n');
		echo ('		if (i != j) {\n');
		if (polyCorr) {
			if(method == "\"polyserial\""){
				comment('polyserial expects x to be numeric', '			');
				echo('			if(is.numeric(data[[i]]) & !is.numeric(data[[j]])){\n');
				echo('				t <- polyserial(data[[i]], data[[j]]');
				if (do_p) {
					echo(', std.err=TRUE');
				} else {}
				echo(')\n			} else if(is.numeric(data[[j]]) & !is.numeric(data[[i]])){\n');
				echo('				t <- polyserial(data[[j]], data[[i]]');
				if (do_p) {
					echo(', std.err=TRUE');
				} else {}
				echo(')\n			} else {\n');
				echo('				next\n');
				echo('			}\n');
			} else {
				echo('			t <- polychor(data[[i]], data[[j]]');
				if (do_p) {
					echo(', std.err=TRUE)\n');
				} else {
					echo(')\n');
				}
			}
			if (do_p) {
				echo ('			result[j, i] <- result[i, j] <- t$rho\n');
				echo ('			result.p[j, i] <- paste("Chisq=", format(t$chisq), ",<br />df=", t$df, ",<br />p=", format(pchisq(t$chisq, t$df, lower.tail=FALSE)), sep="")\n');
				echo ('			result.p[i, j] <- paste("se=", format(sqrt(diag(t$var))), ",<br />n=", t$n, sep="")\n');
			} else {
				echo ('			result[i, j] <- result[j, i] <- t\n');
			}
		} else {
			echo ('			t <- cor.test (data[[i]], data[[j]], method=' + method + ')\n');
			echo ('			result.p[i, j] <- t$p.value\n');
			echo ('			result.p[j, i] <- sum (complete.cases (data[[i]], data[[j]]))\n');
		}
		echo ('		}\n');
		echo ('	}\n');
		echo ('}\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		// TODO: Printing of method and use is a poor solution, esp. when translated. We should support getting the <radio>'s option labels, and print those, instead.
		new Header (i18n ("Correlation Matrix")).addFromUI ("method").addFromUI ("use").print ();
	}
	echo ('rk.results (data.frame (result, check.names=FALSE), titles=c (' + i18n ("Coefficient") + ', names (data)))\n');
	if (do_p) {
		if (polyCorr) {
			new Header (i18n ("Standard errors, test of bivariate normality and sample size"), 4).print ();
			echo ('rk.results (data.frame (result.p, check.names=FALSE, stringsAsFactors=FALSE), titles=c ("Chisq, df, p \\\\ se, n", names (data)))\n');
		} else {
			new Header (i18n ("p-values and sample size"), 4).print ();
			echo ('rk.results (data.frame (result.p, check.names=FALSE), titles=c ("n \\\\ p", names (data)))\n');
		}
	}
	if (!polyCorr && toNumeric) {
		echo ('if(length(transformed.vars) > 0){\n');
		new Header (i18n ("Variables treated as numeric ranks"), 4).print ('\t');
		echo ('	for (i in names(transformed.vars)) {\n');
		echo ('		rk.print(paste(' + i18nc ("noun", "Variable:") + ', "<b>", i, "</b>"))\n');
		echo ('		rk.results(transformed.vars[[i]], titles=c(' + i18n ("original value") + ', ' + i18n ("assigned rank") + '))\n');
		echo ('	}\n');
		echo ('} else {}\n');
	} else {}
}
