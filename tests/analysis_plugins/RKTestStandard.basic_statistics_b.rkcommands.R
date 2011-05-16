local({
## Prepare
## Compute
vars <- list (substitute (test10x), substitute (women[["height"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length (vars))  {
	var <- eval (vars[[i]], envir=globalenv());
	results[i, 'Variable Name'] <- rk.get.description(vars[[i]], is.substitute=TRUE)

	if (length (var) >= 2) {
		results[i, 'Minimum values'] <- paste (sort(var, decreasing=FALSE, na.last=TRUE)[1:2], collapse=" ")
	}
	if (length (var) >= 3) {
		results[i, 'Maximum values'] <- paste (sort(var, decreasing=TRUE, na.last=TRUE)[1:3], collapse=" ")
	}
	
	#robust statistics
}

## Print result
rk.header ("Univariate statistics", parameters=list (
"Remove Missing values", TRUE))

rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::basic_statistics", settings="autre.real=0.00\nhuber.state=\nirq.state=0\nlength.state=0\nmad.state=\nmaximum.state=0\nmean.state=0\nmedian.state=0\nminimum.state=0\nnarm.state=1\nnbmaximum.real=3.00\nnbminimum.real=2.00\nquartile.state=0\nsaveas.active=0\nsaveas.objectname=rk.univariate\nsaveas.parent=.GlobalEnv\nsd.state=0\ntrim.state=\nvari.state=0\nz.available=test10x\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
