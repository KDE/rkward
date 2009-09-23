local({
## Prepare
require(nortest)
## Compute
vars <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- pearson.test (var, adjust = TRUE)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
		results[i, 'number of classes'] <- test$n.classes
		results[i, 'degrees of freedom'] <- test$df
	})
}
## Print result
rk.header ("Pearson chi-square Normality Test",
	parameters=list ("chi-square distribution with n.classes-3 df (TRUE) or chi-square distribution with n.classes-1 df (FALSE)", "adjust = TRUE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::pearson_test", settings="adjust.string=adjust = TRUE\nlength.state=1\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]", label="Run again")
.rk.make.hr()
local({
## Prepare
require(nortest)
## Compute
vars <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- pearson.test (var, adjust = FALSE)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
		results[i, 'number of classes'] <- test$n.classes
		results[i, 'degrees of freedom'] <- test$df
	})
}
## Print result
rk.header ("Pearson chi-square Normality Test",
	parameters=list ("chi-square distribution with n.classes-3 df (TRUE) or chi-square distribution with n.classes-1 df (FALSE)", "adjust = FALSE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::pearson_test", settings="adjust.string=adjust = FALSE\nlength.state=1\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]", label="Run again")
.rk.make.hr()
