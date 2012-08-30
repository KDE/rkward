local({
## Compute
save(testx,
	file="x.RData",
	ascii=TRUE,
	compress="gzip",
	compression_level=6)

## Print result
rk.header("Save R objects", parameters=list("File", "x.RData",
	"Variables", "testx"))
})
local({
## Compute
save(testy,
	file="y.RData",
	compress="xz",
	compression_level=-9)

## Print result
rk.header("Save R objects", parameters=list("File", "y.RData",
	"Variables", "testy"))
})
