local({
## Prepare
require(eRm)
## Compute
estimates.lrsm <- LRSM(lrsmdat, mpoints=2, se=FALSE, sum0=FALSE)
## Print result
rk.header ("LRSM  parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.lrsm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print(t(rbind(Eta=estimates.lrsm$etapar,StdErr=estimates.lrsm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.lrsm$loglik, digits=1),
"<br />Number of iterations:",estimates.lrsm$iter,"<br />Number of parameters:",estimates.lrsm$npar))
# keep results in current workspace
.GlobalEnv$estimates.lrsm <- estimates.lrsm
})
