local({
## Print result
rk.header ("Stripchart", parameters=list("Variable"=rk.get.description (warpbreaks[["breaks"]]),
	"Group"=rk.get.description (warpbreaks[["tension"]]),
	"Treatment of identical values"="Stack",
	"Offset"="0.50",
	"Orientation"="Horizontal"))

rk.graph.on ()
try ({
	stripchart (warpbreaks[["breaks"]] ~ (warpbreaks[["tension"]]), method = "stack", offset = 0.50)
})
rk.graph.off ()
})
