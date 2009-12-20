local({
## Prepare
require(ltm)
## Compute
GoFRasch.res <- GoF.rasch(estimates.rasch)
## Print result
rk.header ("Goodness of Fit for Rasch Models (estimates.rasch)")
rk.print (GoFRasch.res)
})
.rk.rerun.plugin.link(plugin="rkward::ltm_gof_rasch", settings="spin_samples.real=49.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
