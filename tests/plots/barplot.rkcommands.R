local({
## Prepare
## Compute
## Print result
x <- swiss[["Catholic"]]
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
if(!is.matrix(x) && is.data.frame(x)) x <- data.matrix(x)
names(x) <- rownames (swiss)
rk.header ("Barplot", parameters=list ("Variable"=rk.get.description (swiss[["Catholic"]]), "colors"="rainbow", "Type"="juxtaposed", "Legend"="FALSE"))

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
rk.header ("Barplot", parameters=list ("Variable"=rk.get.description (test_table), "colors"="default", "Type"="stacked", "Legend"="TRUE"))

rk.graph.on ()
try ({
	barplot(x, legend.text=TRUE)
})
rk.graph.off ()
})
local({
## Prepare
## Compute
## Print result
groups <- rk.list (warpbreaks[["wool"]], warpbreaks[["tension"]])
title <- paste (names (groups), collapse=" by ")
x <- by (warpbreaks[["breaks"]], interaction (groups), FUN=sum)
rk.header ("Barplot", parameters=list ("Tabulation groups"=paste (names (groups), collapse=" by "), "Tabulation statistic"="Sum of warpbreaks[[\"breaks\"]]", "colors"="rainbow", "Type"="juxtaposed", "Legend"="TRUE"))

rk.graph.on ()
try ({
	# adjust the range so that the labels will fit
	yrange <- range (x, na.rm=TRUE) * 1.2
	if (yrange[1] > 0) yrange[1] <- 0
	if (yrange[2] < 0) yrange[2] <- 0
	bplot <- barplot(x, col=rainbow (if(is.matrix(x)) dim(x) else length(x)), beside=TRUE, legend.text=TRUE, ylim = yrange, xlab=title, ylab="Sum of warpbreaks[[\"breaks\"]]")
	text (bplot,x, labels=x, pos=3, offset=.5)
})
rk.graph.off ()
})
