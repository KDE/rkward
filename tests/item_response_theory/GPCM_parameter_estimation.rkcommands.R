local({
## Prepare
require(ltm)
## Compute
estimates.gpcm <- gpcm(subset(Science, select=c("Work", "Industry", "Future", "Benefit")), constraint="rasch")
## Print result
rk.header ("GPCM parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.gpcm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.gpcm))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.gpcm$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.gpcm <- estimates.gpcm
})
