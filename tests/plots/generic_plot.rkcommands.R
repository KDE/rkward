local({
## Print result
rk.header ("Generic Plot")
rk.graph.on ()
try({
	plot(swiss);
})
rk.graph.off ()
})
