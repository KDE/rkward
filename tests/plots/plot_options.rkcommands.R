local({
## Print result
rk.header ("Generic Plot")
rk.graph.on ()
try({
	par (mar=c (5.0, 4.0, 4.9, 2.0))

	plot(women[["weight"]], type="b", ylim=c (100, 200), main="This is a test", sub="subtitle", las=2, cex.axis=1.1);

	grid(nx=NA, ny=NULL, col="blue", lty="dashed");
})
rk.graph.off ()
})
