local({
## Compute
load (file="PATH/women.RData", envir=globalenv())
## Print result
rk.header ("Load data", parameters=list("R data file to load"="PATH/women.RData",
	"Import to environment"="globalenv()"))
})
