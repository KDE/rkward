local({
## Print result
x <- women[["height"]]
if (!is.numeric (x)) {
	warning ("Data is not numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Dot chart", parameters=list ("Variable"=rk.get.description (women[["height"]])))

rk.graph.on ()
try ({
names(x) <- women$weight
	dotchart(x, main="This is a test", sub="This is a subtitle")
})
rk.graph.off ()
})
local({
## Print result
groups <- rk.list (warpbreaks[["tension"]], warpbreaks[["wool"]])
title <- paste (names (groups), collapse=" by ")
x <- by (warpbreaks[["breaks"]], interaction (groups), FUN=function (x) { mean (x) })
n <- names (x); x <- as.numeric (x); names (x) <- n		# dotchart() is somewhat picky about data type

rk.header ("Dot chart", parameters=list ("Tabulation groups"=paste (names (groups), collapse=" by "), "Tabulation statistic"="mean (x) of warpbreaks[[\"breaks\"]]"))

rk.graph.on ()
try ({
	dotchart(x, xlab="mean (x) of warpbreaks[[\"breaks\"]]", ylab=title)
})
rk.graph.off ()
})
