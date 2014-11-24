local({
## Compute
vars <- rk.list (test10x, women[["height"]])
results <- data.frame ("Variable Name"=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	if (length (var) >= 2) {
		results[i, "Minimum values"] <- paste (format(sort(var, decreasing=FALSE, na.last=TRUE)[1:2]), collapse=" ")
	}
	if (length (var) >= 3) {
		results[i, "Maximum values"] <- paste (format(sort(var, decreasing=TRUE, na.last=TRUE)[1:3]), collapse=" ")
	}
	
	# robust statistics
}

## Print result
rk.header ("Univariate statistics", parameters=list("Omit missing values"="yes"))

rk.results (results)
})
