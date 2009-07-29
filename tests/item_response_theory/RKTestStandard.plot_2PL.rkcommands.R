local({
## Prepare
  require(ltm)
## Compute
## Print result
rk.header("Two parameter logistic model plot")

rk.graph.on()
try(plot(estimates.2pl, type="ICC", items=c(1)))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::plot_ltm", settings="annotation.string=annot\ninp_items.text=1\nplot_type.string=items\nplot_type_item.string=ICC\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nspin_from.real=-3.80000000\nspin_to.real=3.80000000\nx.available=estimates.2pl", label="Run again")
.rk.make.hr()
