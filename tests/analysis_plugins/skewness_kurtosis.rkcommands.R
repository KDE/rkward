local({
## Prepare
require(moments)
## Compute
vars <- rk.list (women[["weight"]], women[["height"]], warpbreaks[["breaks"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Error"] <- tryCatch ({
		# This is the core of the calculation
		results[i, "Skewness"] <- skewness (var, na.rm=TRUE)
		results[i, "Kurtosis"] <- kurtosis (var, na.rm=TRUE)
		results[i, "Excess Kurtosis"] <- results[i, 'Kurtosis'] - 3
		results[i, "Geary Kurtosis"] <- geary (var, na.rm=TRUE)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
	results[i, "Length"] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
}
if (all (is.na (results$"Error"))) results$"Error" <- NULL
## Print result
rk.header ("Skewness and Kurtosis")
rk.results (results)
})
