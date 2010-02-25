// globals
var use;
var method;
var do_p;

function calculate () {
	do_p = getValue ("do_p");

	var exclude_whole = "";
	var vars = "substitute (" + str_replace ("\n", "), substitute (", trim (getValue ("x"))) + ")";
	use = getValue ("use");
	if (use == "pairwise") {
		exclude_whole = false;
		use = "\"pairwise.complete.obs\"";
	} else {
		exclude_whole = true;
		use = "\"complete.obs\"";
	}
	method = "\"" + getValue ("method") + "\"";

	echo ('objects <- list (' + vars + ')\n');
	echo ('\n');
	echo ('# cor requires all objects to be inside the same data.frame.\n');
	echo ('# Here we construct such a temporary frame from the input variables\n');
	echo ('data <- data.frame (lapply (objects, function (x) eval (x, envir=globalenv ())))\n');
	echo ('\n');
	echo ('# calculate correlation matrix\n');
	echo ('result <- cor (data, use=' + use + ', method=' + method + ')\n');
	if (do_p) {
		echo ('# calculate matrix of probabilities\n');
		echo ('result.p <- matrix (nrow = length (data), ncol = length (data))\n');
		if (exclude_whole) {
			echo ('# as we need to do pairwise comparisons for technical reasons,\n');
			echo ('# we need to exclude incomplete cases first to match the use="complete.obs" parameter to cor()\n');
			echo ('data <- data[complete.cases (data),]\n');
		}
		echo ('for (i in 1:length (data)) {\n');
		echo ('	for (j in i:length (data)) {\n');
		echo ('		if (i != j) {\n');
		echo ('			t <- cor.test (data[[i]], data[[j]], method=' + method + ')\n');
		echo ('			result.p[i, j] <- t$p.value\n');
		echo ('			result.p[j, i] <- sum (complete.cases (data[[i]], data[[j]]))\n');
		echo ('		}\n');
		echo ('	}\n');
		echo ('}\n');
	}
}

function printout () {
	echo ('rk.header ("Correlation Matrix", parameters=list ("Method", ' + method + ', "Exclusion", ' + use + '))\n');
	echo ('\n');
	echo ('result <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result))\n');
	echo ('rk.results (result, titles=c (\'Coefficient\', sapply (objects, rk.get.short.name)))\n');
	echo ('\n');
	if (do_p) {
		echo ('result.p <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result.p))\n');
		echo ('rk.results (result.p, titles=c (\'n \\\\ p\', sapply (objects, rk.get.short.name)))\n');
	}
}


