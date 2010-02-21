local({
## Prepare
## Compute
## Print result
x <- test_table[["A"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Pie chart", parameters=list ("Variable", rk.get.description (test_table[["A"]]), "Tabulate", "No", "Clockwise", "Yes"))

rk.graph.on ()
try ({
	pie(x, clockwise =1, density =3 + 1 * 0:length (x), angle =45 + 6 * 0:length (x), col=gray.colors (if(is.matrix(x)) dim(x) else length(x)))
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::piechart", settings="angle.real=45\nangle_inc.real=6\nclockwise.state=1\ncolors.string=grayscale\ndensity.real=3\ndensity_inc.real=1\nnames_exp.text=names (x)\nnames_mode.string=default\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nradius.real=0.80\ntabulate.state=0\nx.available=test_table[[\\\"A\\\"]]", label="Run again")
.rk.make.hr()
