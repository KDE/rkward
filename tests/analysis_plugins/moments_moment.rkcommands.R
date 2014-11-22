local({
## Prepare
require(moments)
## Compute
vars <- rk.list (test50z, test50y, test50x, test10z, test10y, test10x)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Error"] <- tryCatch ({
		# This is the core of the calculation
		results[i, "Moment"] <- moment (var, order = 1, central = FALSE, absolute = FALSE, na.rm = TRUE)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$"Error"))) results$"Error" <- NULL
## Print result
rk.header ("Statistical Moment", parameters=list("Order"="1",
	"compute central moments"="no",
	"compute absolute moments"="no",
	"remove missing values"="yes"))
rk.results (results)
})
