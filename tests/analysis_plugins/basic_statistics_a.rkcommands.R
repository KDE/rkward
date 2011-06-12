local({
## Prepare
## Compute
vars <- rk.list (women[["weight"]], test50x)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	results[i, 'Number of obs'] <- length(var)
	results[i, 'Number of missing values'] <- sum(is.na(var))
	results[i, 'Mean'] <- mean(var,na.rm=TRUE)
	results[i, 'Variance'] <- var(var,na.rm=TRUE)
	results[i, 'Sd'] <- sd(var,na.rm=TRUE)
	results[i, 'Minimum'] <- min(var,na.rm=TRUE)
	results[i, 'Maximum'] <- max(var,na.rm=TRUE)
	results[i, 'Median'] <- median(var,na.rm=TRUE)
	results[i, 'Inter Quartile Range'] <- IQR(var,na.rm=TRUE)
	temp <- quantile (var,na.rm=TRUE)
	results[i, 'Quartiles'] <- paste (names (temp), temp, sep=": ", collapse=" ")
	temp <- quantile (var, probs=seq (0, 1, length.out=6), na.rm=TRUE)
	results[i, 'Quantiles'] <- paste (names (temp), temp, sep=": ", collapse=" ")
	
	#robust statistics
	results[i, 'Trimmed Mean'] <- mean (var, trim=0.05, na.rm=TRUE)
	results[i, 'Median Absolute Deviation'] <- mad (var, constant=1.4628, na.rm=TRUE)
	require ("MASS")
	temp <- list (c('Location Estimate','Mad scale estimate'), c(NA,NA))
	try({
		temp <- hubers (var, k = 1.50,tol=0.07, mu=3, s=,initmu =median(var))
	})
	results[i, 'Huber M-Estimator'] <- paste (temp[[1]], temp[[2]], sep=": ", collapse=" ")
}

# store results
.GlobalEnv$my.data <- results
## Print result
rk.header ("Univariate statistics", parameters=list (
"Remove Missing values", TRUE, "Trimmed value for trimmed mean", "0.05"
, "Constant for the MAD estimation", "1.4628"
, "Winsorized values for Huber estimator", "1.50"
, "Tolerance in Huber estimator", "0.07"
, "Mu for Huber estimator", "3"
, "S for Huber estimator", ""
, "Initial value", "median"
))

rk.results (results)
})
