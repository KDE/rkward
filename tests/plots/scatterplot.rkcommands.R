local({
## Prepare
## Compute
Xvars <- list(women[["weight"]],swiss[["Education"]])
Yvars <- list(women[["height"]],swiss[["Catholic"]])

if (length(Xvars) != length(Yvars)) {
	stop("Unequal number of X and Y variables given")
}
# find range of X/Y values needed
Xrange <- range (c (Xvars), na.rm=TRUE)
Yrange <- range (c (Yvars), na.rm=TRUE)

type <- rep (c ('p'), length.out=length (Xvars));
col <- rep (c ('black', 'red'), length.out=length (Xvars));
cex <- rep (1, length.out=length (Xvars));
pch <- rep (1, length.out=length (Xvars));
## Print result
rk.header ("Scatterplot", parameters = list (
	"X variables"=paste (rk.get.description (women[["weight"]],swiss[["Education"]]), collapse=", "),
	"Y variables"=paste (rk.get.description (women[["height"]],swiss[["Catholic"]]), collapse=", ")))

rk.graph.on()

try ({
	# make frame and axes
	plot(Xrange, Yrange, type="n")
	
	# plot variables one X/Y pair at a time
	for (i in 1:length(Xvars)) {
		points (
			Xvars[[i]],
			Yvars[[i]],
			type = type[[i]],
			col = col[[i]],
			cex = cex[[i]],
			pch = pch[[i]]
		)
	}
})

rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::scatterplot", settings="cex.text=1\ncol.text=c ('black', 'red')\ncolor.string=each\nisCex.string=all\nisPch.string=all\npch.text=1\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\npointtype.string=p\ntype_mode.string=all\nx.available=women[[\\\"weight\\\"]]\\nswiss[[\\\"Education\\\"]]\ny.available=women[[\\\"height\\\"]]\\nswiss[[\\\"Catholic\\\"]]", label="Run again")
.rk.make.hr()
