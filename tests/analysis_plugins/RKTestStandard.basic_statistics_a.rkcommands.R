local({
## Prepare
## Compute
vars <- list (substitute (women[["weight"]]), substitute (test50x))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length (vars))  {
	var <- eval (vars[[i]], envir=globalenv());
	results[i, 'Variable Name'] <- rk.get.description(vars[[i]], is.substitute=TRUE)

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
.rk.rerun.plugin.link(plugin="rkward::basic_statistics", settings="autre.real=6.00\nconstMad.real=1.4628\ncustomMu.state=1\ncustomS.state=1\nhuber.state=1\ninitmu.string=median\nirq.state=1\nlength.state=1\nmad.state=1\nmaximum.state=1\nmean.state=1\nmedian.state=1\nminimum.state=1\nmu.text=3\nnarm.state=1\nnbmaximum.real=0.00\nnbminimum.real=0.00\npourcent.real=0.05\nquartile.state=1\ns.text=\nsaveas.active=1\nsaveas.objectname=my.data\nsaveas.parent=.GlobalEnv\nsd.state=1\ntol.real=0.07\ntrim.state=1\nvari.state=1\nwinsor.real=1.50\nz.available=women[[\\\"weight\\\"]]\\ntest50x", label="Run again")
.rk.make.hr()
