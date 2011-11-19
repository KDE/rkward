local({
## Compute
save (testx, file="x.RData", ascii=TRUE, compress=TRUE)
## Print result
rk.header("Save R objects", parameters=list("File", "x.RData",
	"Variables", "testx"))
})
local({
## Compute
save (testy, file="y.RData", ascii=TRUE, compress=TRUE)
## Print result
rk.header("Save R objects", parameters=list("File", "y.RData",
	"Variables", "testy"))
})
