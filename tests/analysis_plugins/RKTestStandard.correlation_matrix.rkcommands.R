local({
## Prepare
## Compute
objects <- list (substitute (test50x), substitute (test50y), substitute (test50z))

# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data <- data.frame (lapply (objects, function (x) eval (x, envir=globalenv ())))

# calculate correlation matrix
result <- cor (data, use="pairwise.complete.obs", method="pearson")
# calculate matrix of probabilities
result.p <- matrix (nrow = length (data), ncol = length (data))
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

result <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result))
rk.results (result, titles=c ('Coefficient', sapply (objects, rk.get.short.name)))

result.p <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result.p))
rk.results (result.p, titles=c ('n \\ p', sapply (objects, rk.get.short.name)))
})
.rk.rerun.plugin.link(plugin="rkward::corr_matrix", settings="do_p.state=1\nmethod.string=pearson\nuse.string=pairwise\nx.available=test50x\\ntest50y\\ntest50z", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
objects <- list (substitute (women[["weight"]]), substitute (women[["height"]]))

# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data <- data.frame (lapply (objects, function (x) eval (x, envir=globalenv ())))

# calculate correlation matrix
result <- cor (data, use="pairwise.complete.obs", method="pearson")
## Print result
rk.header ("Correlation Matrix", parameters=list ("Method", "pearson", "Exclusion", "pairwise.complete.obs"))

result <- data.frame (I (sapply (objects, FUN=function (x) rk.get.description (x, is.substitute=TRUE))), as.data.frame (result))
rk.results (result, titles=c ('Coefficient', sapply (objects, rk.get.short.name)))

})
.rk.rerun.plugin.link(plugin="rkward::corr_matrix", settings="do_p.state=\nmethod.string=pearson\nuse.string=pairwise\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
