local({
## Prepare
## Compute
vars <- list (substitute (women[["height"]]), substitute (test10z))
results <- data.frame ('Object'=rep (NA, length (vars)))
for (i in 1:length (vars)) {
	results[i, 'Object'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv())	# fetch the real object

	# we wrap each single call in a "try" statement to always continue on errors.
	results[i, 'mean'] <- try (mean (var, trim = 0.00000000, na.rm=TRUE))
	results[i, 'median'] <- try (median (var, na.rm=TRUE))
	try ({
		range <- try (range (var, na.rm=TRUE))
		results[i, 'min'] <- range[1]
		results[i, 'max'] <- range[2]
	})
	results[i, 'standard deviation'] <- try (sd (var, na.rm=TRUE))
	results[i, 'sum'] <- try (sum (var, na.rm=TRUE))
	results[i, 'product'] <- try (prod (var, na.rm=TRUE))
	results[i, 'Median Absolute Deviation'] <- try (mad (var, constant = 1.46280000, na.rm=TRUE))
	results[i, 'length of sample'] <- length (var)
	results[i, 'number of NAs'] <- sum (is.na(var))
}
## Print result
rk.header ("Descriptive statistics", parameters=list (
               "Trim of mean", 0.00000000,
               "Median Absolute Deviation",
               paste ("constant:", 1.46280000, "average")))

rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::descriptive", settings="constMad.real=1.46280000\nlength.state=1\nmad.state=1\nmad_type.string=average\nmean.state=1\nmedian.state=1\nprod.state=1\nrange.state=1\nsd.state=1\nsum.state=1\ntrim.real=0.00000000\nx.available=women[[\\\"height\\\"]]\\ntest10z", label="Run again")
.rk.make.hr()
