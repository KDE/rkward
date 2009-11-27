local({
## Prepare
  require(ltm)
## Compute
cronalpha.res <- cronbach.alpha(subset(LSAT, select=c("Item 1", "Item 2", "Item 3", "Item 4", "Item 5")), CI=TRUE)
## Print result
rk.header ("Cronbach's alpha (LSAT, subset: Item 1, Item 2, Item 3, Item 4, Item 5)")
rk.print (cronalpha.res)
})
.rk.rerun.plugin.link(plugin="rkward::ltm_cronbach_alpha", settings="chk_bsci.state=bsci\nchk_na.state=\nchk_select.state=select\nchk_standard.state=\ninp_items.available=LSAT[[\\\"Item 1\\\"]]\\nLSAT[[\\\"Item 2\\\"]]\\nLSAT[[\\\"Item 3\\\"]]\\nLSAT[[\\\"Item 4\\\"]]\\nLSAT[[\\\"Item 5\\\"]]\nspin_ci.real=0.95\nspin_samples.real=1000.00\nx.available=LSAT", label="Run again")
.rk.make.hr()
