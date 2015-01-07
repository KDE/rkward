local({
## Compute
vars <- rk.list (women[["height"]], test10z)
results <- data.frame ("Object"=I(names (vars)))
for (i in 1:length (vars)) {
	var <- vars[[i]]

	# we wrap each single call in a "try" statement to always continue on errors.
	results[i, "mean"] <- try (mean (var, trim = 0.00, na.rm=TRUE))
	results[i, "median"] <- try (median (var, na.rm=TRUE))
	try ({
		range <- try (range (var, na.rm=TRUE))
		results[i, "min"] <- range[1]
		results[i, "max"] <- range[2]
	})
	results[i, "standard deviation"] <- try (sd (var, na.rm=TRUE))
	results[i, "sum"] <- try (sum (var, na.rm=TRUE))
	results[i, "product"] <- try (prod (var, na.rm=TRUE))
	results[i, "Median Absolute Deviation"] <- try (mad (var, constant = 1.4628, na.rm=TRUE))
	results[i, "length of sample"] <- length (var)
	results[i, "number of NAs"] <- sum (is.na(var))
}
## Print result
rk.header ("Descriptive statistics", parameters=list("Trim of mean"="0.00"))
rk.header ("Median Absolute Deviation", parameters=list("Constant"="1.4628",
	"Treatment of even sample sizes"="use average"), level=3)
rk.results (results)
})
