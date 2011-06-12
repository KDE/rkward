local({
## Prepare
require (nortest)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- lillie.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Lilliefors (Kolmogorov-Smirnov) Normality test")
rk.results (results)
})
