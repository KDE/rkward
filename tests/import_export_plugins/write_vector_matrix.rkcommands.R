local({
## Compute
write (x=testx, file="PATH/data", ncolumns=2, append=FALSE, sep=",")
## Print result
rk.header ("Write Variables", parameters=list("File name"="PATH/data",
	"Data"="testx"))
})
