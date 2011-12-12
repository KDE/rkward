// globals
var use;
var method;
var do_p;
var toNumeric;

function calculate () {
	do_p = getValue ("do_p");

	var exclude_whole = "";
	var vars = trim (getValue ("x"));
	toNumeric = getValue ("to_numeric");
	use = getValue ("use");
	if (use == "pairwise") {
		exclude_whole = false;
		use = "\"pairwise.complete.obs\"";
	} else {
		exclude_whole = true;
		use = "\"complete.obs\"";
	}
	method = "\"" + getValue ("method") + "\"";

	echo ('# cor requires all objects to be inside the same data.frame.\n');
	echo ('# Here we construct such a temporary frame from the input variables\n');
	echo ('data.list <- rk.list (' + vars.split ("\n").join (", ") + ')\n');
	if (toNumeric) {
		echo ('# Non-numeric variables will be treated as ordered data and transformed into numeric ranks\n');
		echo ('transformed.vars <- list()\n');
		echo ('for (i in names(data.list)) {\n');
		echo ('	if(!is.numeric(data.list[[i]])){\n');
		echo ('		before.vars <- as.character(unique(data.list[[i]]))\n');
		echo ('		data.list[[i]] <- xtfrm(data.list[[i]])\n');
		echo ('		after.vars <- unique(data.list[[i]])\n');
		echo ('		names(after.vars) <- before.vars\n');
		echo ('		# Keep track of all transformations\n');
		echo ('		transformed.vars[[i]] <- data.frame(rank=sort(after.vars))\n');
		echo ('	} else {}\n');
		echo ('}\n');
		echo ('# Finally combine the actual data\n');
	} else {}
	echo ('data <- as.data.frame (data.list, check.names=FALSE)\n');
	echo ('\n');
	echo ('# calculate correlation matrix\n');
	echo ('result <- cor (data, use=' + use + ', method=' + method + ')\n');
	if (do_p) {
		echo ('# calculate matrix of probabilities\n');
		echo ('result.p <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))\n');
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
	echo ('rk.results (data.frame (result, check.names=FALSE), titles=c ("Coefficient", names (data)))\n');
	if (do_p) {
		echo ('rk.results (data.frame (result.p, check.names=FALSE), titles=c ("n \\\\ p", names (data)))\n');
	}
	if (toNumeric) {
		echo ('if(length(transformed.vars) > 0){\n');
		echo ('	rk.header("Variables transformed into numeric ranks", level=4)\n');
		echo ('	for (i in names(transformed.vars)) {\n');
		echo ('		rk.print(paste("Variable:<b>", i, "</b>"))\n');
		echo ('		rk.results(transformed.vars[[i]], titles=c("original value", "assigned rank"))\n');
		echo ('	}\n');
		echo ('} else {}\n');
	} else {}
}
