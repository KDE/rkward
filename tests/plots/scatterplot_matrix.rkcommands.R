local({
## Prepare
require(car)
## Print result
data <- data.frame (swiss)

rk.header ("Scatterplot Matrix", parameters=list ("Diagonal Panels", "histogram", "Plot points", "TRUE", "Smooth", "FALSE", "Ellipses", "FALSE at 0.5 and 0.9 levels."))

rk.graph.on ()
try (scatterplotMatrix(data, diagonal="histogram", plot.points=TRUE, smooth=FALSE, ellipse=FALSE))
rk.graph.off ()
})
