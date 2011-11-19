local({
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]], rock[["peri"]], rock[["area"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	try ({
		test <- PP.test (var, lshort = FALSE)
		results[i, 'Dickey-Fuller'] <- test$statistic
		results[i, 'Truncation lag parameter'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Phillips-Perron Test for Unit Roots",
	parameters=list ("Truncation lag parameter short ('TRUE') or long ('FALSE')", "FALSE"))

rk.results (results)
})
