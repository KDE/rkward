local({
## Prepare
## Compute
vars <- rk.list (test50x, test10y)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	try ({
		test <- Box.test (var, lag = 1, type = "Box-Pierce")
		results[i, 'X-squared'] <- test$statistic
		results[i, 'degrees of freedom'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Box-Pierce Test",
	parameters=list ("lag", "1", "type", "Box-Pierce"))

rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::Box_test", settings="lag.real=1.00\nlength.state=1\nnarm.state=0\ntype.string=Box-Pierce\nx.available=test50x\\ntest10y", label="Run again")
.rk.make.hr()
