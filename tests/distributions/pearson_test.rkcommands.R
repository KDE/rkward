local({
## Prepare
require (nortest)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]])
results <- data.frame ("Variable Name"=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Length"] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- pearson.test (var, adjust = TRUE)
		results[i, "Statistic"] <- paste (names (test$statistic), format (test$statistic), sep=" = ")
		results[i, "number of classes"] <- test$n.classes
		results[i, "degrees of freedom"] <- test$df
		results[i, "p-value"] <- test$p.value
	})
}
## Print result
rk.header ("Pearson chi-square Normality Test", parameters=list("p-value from chi^2-distribution from"="n.classes-3 degrees of freedom"))
rk.results (results)
})
local({
## Prepare
require (nortest)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]])
results <- data.frame ("Variable Name"=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Length"] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- pearson.test (var, adjust = FALSE)
		results[i, "Statistic"] <- paste (names (test$statistic), format (test$statistic), sep=" = ")
		results[i, "number of classes"] <- test$n.classes
		results[i, "degrees of freedom"] <- test$df
		results[i, "p-value"] <- test$p.value
	})
}
## Print result
rk.header ("Pearson chi-square Normality Test", parameters=list("p-value from chi^2-distribution from"="n.classes-1 degrees of freedom"))
rk.results (results)
})
