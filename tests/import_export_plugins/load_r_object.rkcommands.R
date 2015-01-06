local({
## Compute
load (file="women.RData", envir=globalenv())
## Print result
rk.header ("Load data", parameters=list("R data file to load"="women.RData",
	"Import to environment"="globalenv()"))
})
