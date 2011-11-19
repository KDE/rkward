local({
## Compute
load (file="women.RData", envir=globalenv())
## Print result
rk.header("Load data", parameters=list("File", "women.RData",
	"Import to environment", "globalenv()"))
})
