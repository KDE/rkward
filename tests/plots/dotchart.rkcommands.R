local({
## Prepare
## Compute
## Print result
x <- women[["height"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (women[["height"]]), "Tabulate", "No"))

rk.graph.on ()
try ({
names(x) <- women$weight
	dotchart(x, main="This is a test", sub="This is a subtitle")
})
rk.graph.off ()
})
local({
## Prepare
## Compute
## Print result
x <- table (warpbreaks[["tension"]], exclude=NULL)

rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (warpbreaks[["tension"]]), "Tabulate", "Yes"))

rk.graph.on ()
try ({
	dotchart(x)
})
rk.graph.off ()
})
