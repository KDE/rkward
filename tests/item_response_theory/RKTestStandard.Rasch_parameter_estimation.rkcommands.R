local({
## Prepare
  require(ltm)
## Compute
estimates.rasch <- rasch(LSAT)
## Print result
rk.header ("Rasch parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.rasch$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print (coef(estimates.rasch))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.rasch$log.Lik, digits=1)))
# keep results in current workspace
estimates.rasch <<- estimates.rasch
})
.rk.rerun.plugin.link(plugin="rkward::par_est_rasch", settings="chk_save.state=save\nconstraint.available=\nghk_rasch.real=21.00\nirtparam.state=TRUE\niterqn_rasch.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nsave_name.selection=estimates.rasch\nstartval.string=NULL\nverbose.state=\nx.available=LSAT", label="Run again")
.rk.make.hr()
