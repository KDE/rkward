local({
## Prepare
## Compute
## Print result
rk.header ("Histogram", list ("Variable", rk.get.description (swiss[["Education"]]) , "Density bandwidth", "nrd", "Density adjust", 4.00000000, "Density resolution", 512.00000000, "Density Remove missing values", na.rm=TRUE  , "Break points", "Equally spaced vector of length 6", "Right closed", "TRUE", "Include in lowest cell", "TRUE", "Scale", "Density"))

rk.graph.on ()
try ({
	hist (swiss[["Education"]], breaks=(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=6)})(swiss[["Education"]]), freq=FALSE, labels=TRUE, lty="solid", density=-1)
	lines(density(swiss[["Education"]], bw="nrd", adjust = 4.00000000, na.rm=TRUE, n = 512.00000000), col="blue")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::histogram", settings="adjust.real=4.00000000\nbw.string=nrd\ncol_density.color.string=blue\ndensity.state=1\nhistogram_opt.addtoplot.state=\nhistogram_opt.barlabels.state=1\nhistogram_opt.density.real=-1.000000\nhistogram_opt.doborder.state=1\nhistogram_opt.freq.state=0\nhistogram_opt.histbordercol.color.string=\nhistogram_opt.histbreaksFunction.string=vec\nhistogram_opt.histbreaks_veclength.real=6.000000\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.include_lowest.state=1\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=\nn.real=512.00000000\nnarm.state=na.rm=TRUE\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nx.available=swiss[[\\\"Education\\\"]]", label="Run again")
.rk.make.hr()
