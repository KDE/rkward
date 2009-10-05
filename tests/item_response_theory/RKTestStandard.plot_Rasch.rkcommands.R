local({
## Prepare
  require(ltm)
## Compute
## Print result
rk.header("Rasch model plot")

rk.graph.on()
try(plot(estimates.rasch, type="ICC", legend=TRUE))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::plot_rasch", settings="annotation.string=legend\ninp_items.text=\nplot_type.string=items\nplot_type_item.string=ICC\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nspin_from.real=-3.80\nspin_to.real=3.80\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
