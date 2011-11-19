local({
## Print result
rk.header ("Hypergeometric distribution function", list ("Lower quantile", "0", "Upper quantile", "12", "Number of white balls", "12", "Number of black balls", "12", "Number of balls drawn", "15", "Scale", "normal", "Tail","Lower", "Function", "phyper"));

rk.graph.on ()
try ({
	curve (phyper(x, m=12, n=12, k=15, lower.tail = TRUE), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
