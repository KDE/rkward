local({
## Compute
write (x=testx, file="data", ncolumns=2, append=FALSE, sep=",")
## Print result
rk.header ("Write Variables", parameters=list("File name"="data",
	"Data"="testx"))
})
