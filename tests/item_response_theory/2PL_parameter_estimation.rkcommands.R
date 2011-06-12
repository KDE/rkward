local({
## Prepare
require(ltm)
## Compute
estimates.2pl <- ltm(WIRS ~ z1 * z2)
## Print result
rk.header ("2PL parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.2pl$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.2pl))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.2pl$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.2pl <- estimates.2pl
})
