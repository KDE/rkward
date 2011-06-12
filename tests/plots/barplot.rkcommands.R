local({
## Prepare
## Compute
## Print result
x <- swiss[["Catholic"]]
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
if(!is.matrix(x) && is.data.frame(x)) x <- data.matrix(x)
names(x) <- rownames (swiss)
rk.header ("Barplot", parameters=list ("Variable", rk.get.description (swiss[["Catholic"]]), "Tabulate", "No", "colors", "rainbow", "Type", "juxtaposed", "Legend", "FALSE"))

rk.graph.on ()
try ({
	# adjust the range so that the labels will fit
	yrange <- range (x, na.rm=TRUE) * 1.2
	if (yrange[1] > 0) yrange[1] <- 0
	if (yrange[2] < 0) yrange[2] <- 0
	bplot <- barplot(x, col=rainbow (if(is.matrix(x)) dim(x) else length(x)), beside=TRUE, ylim = yrange)
	text (bplot,x, labels=x, pos=3, offset=.5)
})
rk.graph.off ()
})
local({
## Prepare
## Compute
## Print result
x <- test_table
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
if(!is.matrix(x) && is.data.frame(x)) x <- data.matrix(x)
rk.header ("Barplot", parameters=list ("Variable", rk.get.description (test_table), "Tabulate", "No", "colors", "default", "Type", "stacked", "Legend", "TRUE"))

rk.graph.on ()
try ({
	barplot(x, legend.text=TRUE)
})
rk.graph.off ()
})
