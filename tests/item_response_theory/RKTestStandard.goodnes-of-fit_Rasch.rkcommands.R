local({
## Prepare
require(ltm)
## Compute
GoFRasch.res <- GoF.rasch(estimates.rasch)
## Print result
rk.header ("Goodness of Fit for Rasch Models (estimates.rasch)")
rk.print ("Call:")
rk.print.literal (deparse(GoFRasch.res$call, width.cutoff=500))
rk.header ("Parametric Bootstrap test:", level=4)
rk.print (paste("Chi-squared statistic:", round(GoFRasch.res$Tobs, digits=3)))
rk.print (paste("Bootstrap samples:", GoFRasch.res$B))
rk.print (paste("p-value:", GoFRasch.res$p.value))
})
.rk.rerun.plugin.link(plugin="rkward::ltm_gof_rasch", settings="spin_samples.real=49.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
