local({
## Prepare
require(hdrcde)
## Print result
rk.header ("Highest density regions", list ("Variable", rk.get.description (women[["height"]]), "Band Width", "nrd0", "Adjust", 1.00, "Remove Missing Values", na.rm=TRUE, "Length", length (women[["height"]]), "Resolution", 512.00, "Smoothing Kernel", "gaussian"))

rk.graph.on ()
try ({
	hdr.den(den=density(women[["height"]], bw="nrd0", adjust=1.00, kern="gaussian", n=512.00, na.rm=TRUE))
})
rk.graph.off ()
})
