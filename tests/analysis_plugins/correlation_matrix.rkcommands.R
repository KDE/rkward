local({
## Compute
# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data.list <- rk.list (test50x, test50y, test50z)
data <- as.data.frame (data.list, check.names=FALSE)

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

rk.results (data.frame (result, check.names=FALSE), titles=c ("Coefficient", names (data)))
rk.header ("p-values and sample size", level=4)
rk.results (data.frame (result.p, check.names=FALSE), titles=c ("n \\ p", names (data)))
})
local({
## Prepare
require(polycor)
## Compute
# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data.list <- rk.list (test10y, test10a)
data <- as.data.frame (data.list, check.names=FALSE)

# calculate correlation matrix
result <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))
# calculate matrix of probabilities
result.p <- matrix (nrow = length (data), ncol = length (data), dimnames=list (names (data), names (data)))
for (i in 1:length (data)) {
	for (j in i:length (data)) {
		if (i != j) {
			t <- polychor(data[[i]], data[[j]])
			result[i, j] <- result[j, i] <- t
		}
	}
}
## Print result
rk.header ("Correlation Matrix", parameters=list ("Method", "polychoric", "Exclusion", "pairwise"))

rk.results (data.frame (result, check.names=FALSE), titles=c ("Coefficient", names (data)))
})
local({
## Compute
# cor requires all objects to be inside the same data.frame.
# Here we construct such a temporary frame from the input variables
data.list <- rk.list (women[["weight"]], women[["height"]])
data <- as.data.frame (data.list, check.names=FALSE)

# calculate correlation matrix
result <- cor (data, use="pairwise.complete.obs", method="pearson")
## Print result
rk.header ("Correlation Matrix", parameters=list ("Method", "pearson", "Exclusion", "pairwise.complete.obs"))

rk.results (data.frame (result, check.names=FALSE), titles=c ("Coefficient", names (data)))
})
