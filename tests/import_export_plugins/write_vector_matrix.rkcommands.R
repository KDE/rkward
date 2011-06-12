local({
## Prepare
## Compute
write (x=testx, file="data", ncolumns=2, append=FALSE, sep=",")
## Print result
rk.header("Write Variables", parameters=list("File", "data",
	"Data", "testx"))
})
