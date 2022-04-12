local({
## Prepare
require(car)
## Print result
data <- data.frame (swiss)

rk.header ("Scatterplot Matrix", parameters=list("Diagonal panels"="Histogram",
	"Plot points"="yes",
	"Plot smooth"="no",
	"Plot data concentration ellipses"="no"))

rk.graph.on ()
try (scatterplotMatrix(data, diagonal=list(method="histogram"), plot.points=TRUE, smooth=FALSE, ellipse=FALSE))
rk.graph.off ()
})
