local({
## Prepare
## Compute
x <- warpbreaks[["tension"]]
yvars <- list (substitute (warpbreaks[["wool"]]), substitute (warpbreaks[["tension"]]))
results <- list()
descriptions <- list ()

# calculate crosstabs
for (i in 1:length (yvars)) {
	yvar <- eval (yvars[[i]], envir=globalenv ())
	results[[i]] <- table(x, yvar)

	descriptions[[i]] <- list ('Dependent'=rk.get.description (warpbreaks[["tension"]]), 'Independent'=rk.get.description (yvars[[i]], is.substitute=TRUE))
}

# calculate chisquares
chisquares <- list ()
for (i in 1:length (results)) {
	chisquares[[i]] <- chisq.test (results[[i]], simulate.p.value = FALSE)
}
## Print result
rk.header ("Crosstabs (n to 1)", level=1)
for (i in 1:length (results)) {
	rk.header ("Crosstabs (n to 1)", parameters=list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']]), level=2)
	rk.results (results[[i]], titles=c(descriptions[[i]][['Dependent']], descriptions[[i]][['Independent']]))

	rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']], "Method", chisquares[[i]][["method"]]), level=2)
	rk.results (list ('Statistic'=chisquares[[i]][['statistic']], 'df'=chisquares[[i]][['parameter']], 'p'=chisquares[[i]][['p.value']]))

	rk.header ("Barplot for Crosstabs", list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']], "colors", "default", "Type", "juxtaposed", "Legend", "FALSE"), level=2)
	rk.graph.on ()
	try ({
		barplot(results[[i]], beside=TRUE)
	})
	rk.graph.off ()
}
})
.rk.rerun.plugin.link(plugin="rkward::crosstab", settings="barplot.state=TRUE\nbarplot_embed.colors.string=default\nbarplot_embed.labels.state=0\nbarplot_embed.legend.state=0\nbarplot_embed.plotoptions.add_grid.state=0\nbarplot_embed.plotoptions.asp.real=0.00000000\nbarplot_embed.plotoptions.main.text=\nbarplot_embed.plotoptions.pointcolor.color.string=\nbarplot_embed.plotoptions.pointtype.string=\nbarplot_embed.plotoptions.sub.text=\nbarplot_embed.plotoptions.xaxt.state=\nbarplot_embed.plotoptions.xlab.text=\nbarplot_embed.plotoptions.xlog.state=\nbarplot_embed.plotoptions.xmaxvalue.text=\nbarplot_embed.plotoptions.xminvalue.text=\nbarplot_embed.plotoptions.yaxt.state=\nbarplot_embed.plotoptions.ylab.text=\nbarplot_embed.plotoptions.ylog.state=\nbarplot_embed.plotoptions.ymaxvalue.text=\nbarplot_embed.plotoptions.yminvalue.text=\nbarplot_embed.type.string=juxtaposed\nchisq.state=TRUE\nsimpv.string=FALSE\nx.available=warpbreaks[[\\\"tension\\\"]]\ny.available=warpbreaks[[\\\"wool\\\"]]\\nwarpbreaks[[\\\"tension\\\"]]", label="Run again")
.rk.make.hr()
