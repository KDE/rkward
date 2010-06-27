local({
## Prepare
require(ltm)
## Compute
estimates.3pl <- tpm(LSAT)
## Print result
rk.header ("3PL parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.3pl$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.3pl))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.3pl$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.3pl <- estimates.3pl
})
.rk.rerun.plugin.link(plugin="rkward::par_est_3pl", settings="constraint.available=\nepshess.real=0.001\nghk_3pl.real=21.00\nirtparam.state=TRUE\niterqn_3pl.real=1000.00\nmaxguess.real=1.00\nnaaction.state=\noptimeth.string=BFGS\noptimizer.string=optim\nsave_name.active=1\nsave_name.objectname=estimates.3pl\nsave_name.parent=.GlobalEnv\nstartval.string=NULL\ntype.state=\nverbose.state=\nx.available=LSAT", label="Run again")
.rk.make.hr()
