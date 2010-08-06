local({
## Prepare
require(eRm)
## Compute
.GlobalEnv$fitctrl <- "optim"
waldtest.res <- Waldtest(estimates.rsm)
rm(fitctrl, envir=.GlobalEnv)
## Print result
rk.header ("Wald test (estimates.rsm)")
rk.print ("Call:")
rk.print.literal (deparse(waldtest.res$call, width.cutoff=500))
rk.header ("Wald test on item level (z-values):", level=4)
rk.print(waldtest.res$coef.table)
})
.rk.rerun.plugin.link(plugin="rkward::eRm_waldtest", settings="drop_optimizer.string=optim\nrad_splitcr.string=median\nx.available=estimates.rsm", label="Run again")
.rk.make.hr()
