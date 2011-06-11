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
.rk.rerun.plugin.link(plugin="rkward::barplot", settings="barplot_embed.colors.string=rainbow\nbarplot_embed.labels.state=1\nbarplot_embed.legend.state=0\nbarplot_embed.place.string=3\nbarplot_embed.plotoptions.add_grid.state=0\nbarplot_embed.plotoptions.asp.real=0.00\nbarplot_embed.plotoptions.main.text=\nbarplot_embed.plotoptions.pointcolor.color.string=\nbarplot_embed.plotoptions.pointtype.string=\nbarplot_embed.plotoptions.sub.text=\nbarplot_embed.plotoptions.xaxt.state=\nbarplot_embed.plotoptions.xlab.text=\nbarplot_embed.plotoptions.xlog.state=\nbarplot_embed.plotoptions.xmaxvalue.text=\nbarplot_embed.plotoptions.xminvalue.text=\nbarplot_embed.plotoptions.yaxt.state=\nbarplot_embed.plotoptions.ylab.text=\nbarplot_embed.plotoptions.ylog.state=\nbarplot_embed.plotoptions.ymaxvalue.text=\nbarplot_embed.plotoptions.yminvalue.text=\nbarplot_embed.type.string=juxtaposed\nnames_exp.text=rownames (swiss)\nnames_mode.string=rexp\ntabulate.state=0\nx.available=swiss[[\\\"Catholic\\\"]]", label="Run again")
.rk.make.hr()
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
.rk.rerun.plugin.link(plugin="rkward::barplot", settings="barplot_embed.colors.string=default\nbarplot_embed.legend.state=1\nbarplot_embed.plotoptions.add_grid.state=0\nbarplot_embed.plotoptions.asp.real=0.00\nbarplot_embed.plotoptions.main.text=\nbarplot_embed.plotoptions.pointcolor.color.string=\nbarplot_embed.plotoptions.pointtype.string=\nbarplot_embed.plotoptions.sub.text=\nbarplot_embed.plotoptions.xaxt.state=\nbarplot_embed.plotoptions.xlab.text=\nbarplot_embed.plotoptions.xlog.state=\nbarplot_embed.plotoptions.xmaxvalue.text=\nbarplot_embed.plotoptions.xminvalue.text=\nbarplot_embed.plotoptions.yaxt.state=\nbarplot_embed.plotoptions.ylab.text=\nbarplot_embed.plotoptions.ylog.state=\nbarplot_embed.plotoptions.ymaxvalue.text=\nbarplot_embed.plotoptions.yminvalue.text=\nbarplot_embed.type.string=stacked\nnames_exp.text=names (x)\nnames_mode.string=default\ntabulate.state=0\nx.available=test_table", label="Run again")
.rk.make.hr()
