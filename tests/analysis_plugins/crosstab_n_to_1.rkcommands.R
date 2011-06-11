local({
## Prepare
## Compute
x <- rk.list (warpbreaks[["tension"]])
yvars <- rk.list (warpbreaks[["wool"]], warpbreaks[["tension"]])
results <- list()
chisquares <- list ()

# calculate crosstabs
for (i in 1:length (yvars)) {
	count <- table(x[[1]], yvars[[i]])
	chisquares[[i]] <- chisq.test (count, simulate.p.value = FALSE)
	results[[i]] <- count
}
## Print result
rk.header ("Crosstabs (n to 1)", level=1)
for (i in 1:length (results)) {
	rk.header ("Crosstabs (n to 1)", parameters=list ("Dependent", names (x)[1], "Independent", names (yvars)[i]), level=2)
	rk.results (results[[i]], titles=c(names (x)[1], names (yvars)[i]))

	rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "Method", chisquares[[i]][["method"]]), level=2)
	rk.results (list ('Statistic'=chisquares[[i]][['statistic']], 'df'=chisquares[[i]][['parameter']], 'p'=chisquares[[i]][['p.value']]))

	rk.header ("Barplot for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "colors", "default", "Type", "juxtaposed", "Legend", "FALSE"), level=2)
	rk.graph.on ()
	try ({
		counts <- results[[i]]
		barplot(counts, beside=TRUE)
	})
	rk.graph.off ()
}
})
.rk.rerun.plugin.link(plugin="rkward::crosstab", settings="barplot.state=TRUE\nbarplot_embed.colors.string=default\nbarplot_embed.labels.state=0\nbarplot_embed.legend.state=0\nbarplot_embed.plotoptions.add_grid.state=0\nbarplot_embed.plotoptions.asp.real=0.00\nbarplot_embed.plotoptions.main.text=\nbarplot_embed.plotoptions.pointcolor.color.string=\nbarplot_embed.plotoptions.pointtype.string=\nbarplot_embed.plotoptions.sub.text=\nbarplot_embed.plotoptions.xaxt.state=\nbarplot_embed.plotoptions.xlab.text=\nbarplot_embed.plotoptions.xlog.state=\nbarplot_embed.plotoptions.xmaxvalue.text=\nbarplot_embed.plotoptions.xminvalue.text=\nbarplot_embed.plotoptions.yaxt.state=\nbarplot_embed.plotoptions.ylab.text=\nbarplot_embed.plotoptions.ylog.state=\nbarplot_embed.plotoptions.ymaxvalue.text=\nbarplot_embed.plotoptions.yminvalue.text=\nbarplot_embed.type.string=juxtaposed\nchisq.state=TRUE\nchisq_expected.state=FALSE\nmargins.state=FALSE\nprop_column.state=FALSE\nprop_row.state=FALSE\nprop_total.state=FALSE\nsimpv.string=FALSE\nx.available=warpbreaks[[\\\"tension\\\"]]\ny.available=warpbreaks[[\\\"wool\\\"]]\\nwarpbreaks[[\\\"tension\\\"]]", label="Run again")
.rk.make.hr()
local({
## Prepare
# convenience function to bind together several two dimensional tables into a single three dimensional table
bind.tables <- function (...) {
	tables <- list (...)
	output <- unlist (tables)
	dim (output) <- c (dim (tables[[1]]), length (tables))
	dimnames (output) <- c (dimnames (tables[[1]]), list (statistic=names(tables)))
	output
}
## Compute
x <- rk.list (warpbreaks[["tension"]])
yvars <- rk.list (warpbreaks[["wool"]], warpbreaks[["tension"]])
results <- list()
chisquares <- list ()

# calculate crosstabs
for (i in 1:length (yvars)) {
	count <- table(x[[1]], yvars[[i]])
	chisquares[[i]] <- chisq.test (count, simulate.p.value = FALSE)
	results[[i]] <- bind.tables ("count"=addmargins (count),
		"% of row"=addmargins (prop.table(count, 1) * 100, quiet=TRUE, FUN=function(x) NA),
		"% of column"=addmargins (prop.table(count, 2) * 100, quiet=TRUE, FUN=function(x) NA),
		"% of total"=addmargins (prop.table(count) * 100),
		"expected"=addmargins (chisquares[[i]]$expected, quiet=TRUE, FUN=function(x) NA))
}
## Print result
rk.header ("Crosstabs (n to 1)", level=1)
for (i in 1:length (results)) {
	rk.header ("Crosstabs (n to 1)", parameters=list ("Dependent", names (x)[1], "Independent", names (yvars)[i]), level=2)
	rk.print (ftable (results[[i]], col.vars=2))

	rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "Method", chisquares[[i]][["method"]]), level=2)
	rk.results (list ('Statistic'=chisquares[[i]][['statistic']], 'df'=chisquares[[i]][['parameter']], 'p'=chisquares[[i]][['p.value']]))

	rk.header ("Barplot for Crosstabs", list ("Dependent", names (x)[1], "Independent", names (yvars)[i], "colors", "rainbow", "Type", "juxtaposed", "Legend", "FALSE"), level=2)
	rk.graph.on ()
	try ({
		counts <- results[[i]][, , "count"]
		# adjust the range so that the labels will fit
		yrange <- range (counts, na.rm=TRUE) * 1.2
		if (yrange[1] > 0) yrange[1] <- 0
		if (yrange[2] < 0) yrange[2] <- 0
		bplot <- barplot(counts, col=rainbow (if(is.matrix(counts)) dim(counts) else length(counts)), beside=TRUE, ylim = yrange)
		text (bplot,counts, labels=counts, pos=3, offset=.5)
	})
	rk.graph.off ()
}
})
.rk.rerun.plugin.link(plugin="rkward::crosstab", settings="barplot.state=TRUE\nbarplot_embed.colors.string=rainbow\nbarplot_embed.labels.state=1\nbarplot_embed.legend.state=0\nbarplot_embed.place.string=3\nbarplot_embed.plotoptions.add_grid.state=0\nbarplot_embed.plotoptions.asp.real=0.00\nbarplot_embed.plotoptions.main.text=\nbarplot_embed.plotoptions.pointcolor.color.string=\nbarplot_embed.plotoptions.pointtype.string=\nbarplot_embed.plotoptions.sub.text=\nbarplot_embed.plotoptions.xaxt.state=\nbarplot_embed.plotoptions.xlab.text=\nbarplot_embed.plotoptions.xlog.state=\nbarplot_embed.plotoptions.xmaxvalue.text=\nbarplot_embed.plotoptions.xminvalue.text=\nbarplot_embed.plotoptions.yaxt.state=\nbarplot_embed.plotoptions.ylab.text=\nbarplot_embed.plotoptions.ylog.state=\nbarplot_embed.plotoptions.ymaxvalue.text=\nbarplot_embed.plotoptions.yminvalue.text=\nbarplot_embed.type.string=juxtaposed\nchisq.state=TRUE\nchisq_expected.state=TRUE\nmargins.state=TRUE\nprop_column.state=TRUE\nprop_row.state=TRUE\nprop_total.state=TRUE\nsimpv.string=FALSE\nx.available=warpbreaks[[\\\"tension\\\"]]\ny.available=warpbreaks[[\\\"wool\\\"]]\\nwarpbreaks[[\\\"tension\\\"]]", label="Run again")
.rk.make.hr()
