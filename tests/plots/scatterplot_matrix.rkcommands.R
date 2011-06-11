local({
## Prepare
require(car)
## Compute
## Print result
data <- data.frame (swiss)

rk.header ("Scatterplot Matrix", parameters=list ("Diagonal Panels", "histogram", "Plot points", "TRUE", "Smooth", "FALSE", "Ellipses", "FALSE at 0.5 and 0.9 levels."))

rk.graph.on ()
try (scatterplot.matrix(data, diagonal="histogram", plot.points=TRUE, smooth=FALSE, ellipse=FALSE))
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::scatterplot_matrix", settings="diag.string=histogram\nellipse.state=FALSE\nplot_points.state=TRUE\nsmooth.state=FALSE\nx.available=swiss", label="Run again")
.rk.make.hr()
