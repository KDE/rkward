local({
## Prepare
require(hdrcde)
## Print result
rk.header ("Highest density regions", parameters=list("Variable"=rk.get.description (women[["height"]]),
	"Length"=length (women[["height"]]),
	"Adjust Bandwidth"="1.00",
	"Remove Missing Values"="yes",
	"Resolution"="512.00",
	"Smoothing Kernel"="gaussian (default)",
	"Bandwidth"="nrd0"))

rk.graph.on ()
try ({
	hdr.den(den=density(women[["height"]], bw="nrd0", adjust=1.00, kern="gaussian", n=512.00, na.rm=TRUE))
})
rk.graph.off ()
})
