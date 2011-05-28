local({
## Prepare
## Compute
# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data <- as.data.frame (rk.list (test50x, test50y, test50z), check.names=FALSE)

# calculate correlation matrix
result <- cor (data, use="pairwise.complete.obs", method="pearson")
# calculate matrix of probabilities
result.p <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))
for (i in 1:length (data)) {
	for (j in i:length (data)) {
		if (i != j) {
			t <- cor.test (data[[i]], data[[j]], method="pearson")
			result.p[i, j] <- t$p.value
			result.p[j, i] <- sum (complete.cases (data[[i]], data[[j]]))
		}
	}
}
## Print result
rk.header ("Correlation Matrix", parameters=list ("Method", "pearson", "Exclusion", "pairwise.complete.obs"))

rk.results (data.frame ("Coefficient"=I(names (data)), result, check.names=FALSE))
rk.results (data.frame ("n \\ p"=I(names (data)), result.p, check.names=FALSE))
})
.rk.rerun.plugin.link(plugin="rkward::corr_matrix", settings="do_p.state=1\nmethod.string=pearson\nuse.string=pairwise\nx.available=test50x\\ntest50y\\ntest50z", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data <- as.data.frame (rk.list (women[["weight"]], women[["height"]]), check.names=FALSE)

# calculate correlation matrix
result <- cor (data, use="pairwise.complete.obs", method="pearson")
## Print result
rk.header ("Correlation Matrix", parameters=list ("Method", "pearson", "Exclusion", "pairwise.complete.obs"))

rk.results (data.frame ("Coefficient"=I(names (data)), result, check.names=FALSE))
})
.rk.rerun.plugin.link(plugin="rkward::corr_matrix", settings="do_p.state=\nmethod.string=pearson\nuse.string=pairwise\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
