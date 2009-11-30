local({
## Prepare
  require(ltm)
## Compute
estimates.3pl <- tpm(LSAT)
## Print result
rk.header ("3PL parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.3pl$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print (coef(estimates.3pl))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.3pl$log.Lik, digits=1)))
# keep results in current workspace
estimates.3pl <<- estimates.3pl
})
.rk.rerun.plugin.link(plugin="rkward::par_est_3pl", settings="chk_save.state=save\nconstraint.available=\nepshess.real=0.001\nghk_3pl.real=21.00\nirtparam.state=TRUE\niterqn_3pl.real=1000.00\nmaxguess.real=1.00\nnaaction.state=\noptimeth.string=BFGS\noptimizer.string=optim\nsave_name.selection=estimates.3pl\nstartval.string=NULL\ntype.state=\nverbose.state=\nx.available=LSAT", label="Run again")
.rk.make.hr()
