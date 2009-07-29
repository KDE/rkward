local({
## Prepare
  require(ltm)
## Compute
itemfit.res <- item.fit(estimates.rasch)
## Print result
rk.header ("Item-fit statistics (estimates.rasch)")
rk.print (itemfit.res)
})
.rk.rerun.plugin.link(plugin="rkward::ltm_item_fit", settings="drop_sumgroups.string=median\nrad_pvalue.string=chi2\nspin_groups.real=10.000000\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
