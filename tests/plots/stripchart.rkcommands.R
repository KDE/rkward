local({
## Prepare
## Compute
## Print result
rk.header ("Stripchart", list ("Variable"=rk.get.description (warpbreaks[["breaks"]]), "Group"=rk.get.description (warpbreaks[["tension"]]), "Method"="stack", "Offset" = 0.50, "Orientation"="Horizontal"))

rk.graph.on ()
try (stripchart (warpbreaks[["breaks"]] ~ (warpbreaks[["tension"]]), method = "stack", offset = 0.50))
rk.graph.off ()
})
