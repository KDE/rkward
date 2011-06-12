local({
## Prepare
## Compute
## Print result
x <- test_table[["A"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Pie chart", parameters=list ("Variable", rk.get.description (test_table[["A"]]), "Tabulate", "No", "Clockwise", "Yes"))

rk.graph.on ()
try ({
	pie(x, clockwise =1, density =3 + 1 * 0:length (x), angle =45 + 6 * 0:length (x), col=gray.colors (if(is.matrix(x)) dim(x) else length(x)))
})
rk.graph.off ()
})
