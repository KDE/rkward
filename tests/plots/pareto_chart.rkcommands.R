local({
## Prepare
require(qcc)
## Compute
x <- swiss[["Education"]]
title <- rk.get.description (swiss[["Education"]])
if (!is.numeric (x)) {
	warning ("Data is not numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}
max.categories <- 10
if (length (x) > max.categories) {
	x <- sort (x, decreasing=TRUE)
	x <- x[1:max.categories]
}
## Print result
rk.header ("Pareto chart", parameters=list ("Variable"=title, "Limit"="10 largest values"))

rk.graph.on ()
try ({
	descriptives <- pareto.chart(x, ylab="Frequency", main=title)
	rk.results(descriptives, titles=c(NA,NA))
})
rk.graph.off ()
})
local({
## Prepare
require(qcc)
## Compute
groups <- rk.list (warpbreaks[["wool"]], warpbreaks[["tension"]])
title <- paste (names (groups), collapse=" by ")
x <- by (warpbreaks[["breaks"]], interaction (groups), FUN=sum)
## Print result
rk.header ("Pareto chart", parameters=list ("Tabulation groups"=paste (names (groups), collapse=" by "), "Tabulation statistic"="Sum of warpbreaks[[\"breaks\"]]"))

rk.graph.on ()
try ({
	descriptives <- pareto.chart(x, ylab="Sum of warpbreaks[[\"breaks\"]]", main=title)
	rk.results(descriptives, titles=c(NA,NA))
})
rk.graph.off ()
})
