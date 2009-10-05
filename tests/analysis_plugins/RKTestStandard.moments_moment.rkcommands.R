local({
## Prepare
require(moments)
## Compute

objects <- list (substitute (test50z), substitute (test50y), substitute (test50x), substitute (test10z), substitute (test10y), substitute (test10x))
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)

	try (results[i, 'Moment'] <- moment (var, order = 1, central = FALSE, absolute = FALSE, na.rm = TRUE))
}
## Print result
rk.header ("Statistical Moment",
	parameters=list ("Order", "1", "Compute central moments", "FALSE", "Compute absolute moments", "FALSE", "Remove missing values", "TRUE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::moment", settings="absolute.state=FALSE\ncentral.state=FALSE\nlength.state=0\nnarm.state=TRUE\norder.real=1.00\nx.available=test50z\\ntest50y\\ntest50x\\ntest10z\\ntest10y\\ntest10x", label="Run again")
.rk.make.hr()
