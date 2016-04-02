local({
## Compute
save(testx,
	file="PATH/x.RData",
	ascii=TRUE,
	compress="gzip",
	compression_level=6)

## Print result
rk.header ("Save R objects", parameters=list("File name"="PATH/x.RData",
	"Object"="testx"))
})
local({
## Compute
save(testy,
	file="PATH/y.RData",
	compress="xz",
	compression_level=-9)

## Print result
rk.header ("Save R objects", parameters=list("File name"="PATH/y.RData",
	"Object"="testy"))
})
