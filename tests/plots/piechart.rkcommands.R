local({
## Prepare
## Compute
## Print result
x <- test_table[["A"]]
if (!is.numeric (x)) {
	warning ("Data is not numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Pie chart", parameters=list ("Variable"=rk.get.description (test_table[["A"]]), "Orientation"="Clockwise"))

rk.graph.on ()
try ({
	pie(x, clockwise =1, density =3 + 1 * 0:length (x), angle =45 + 6 * 0:length (x), col=gray.colors (if(is.matrix(x)) dim(x) else length(x)))
})
rk.graph.off ()
})
local({
## Prepare
## Compute
## Print result
groups <- rk.list (test_table[["A"]], test_table[["B"]], test_table[["C"]])
title <- paste (names (groups), collapse=" by ")
x <- table (interaction (groups))

rk.header ("Pie chart", parameters=list ("Tabulation groups"=paste (names (groups), collapse=" by "), "Tabulation statistic"="Frequency", "Orientation"="Counter clockwise"))

rk.graph.on ()
try ({
	pie(x, clockwise =0, main=title, sub="Frequency")
})
rk.graph.off ()
})
