local({
## Prepare
require(eRm)
## Compute
estimates.lpcm <- LPCM(lpcmdat, mpoints=2, groupvec=G)
## Print result
rk.header ("LPCM  parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.lpcm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print(t(rbind(Eta=estimates.lpcm$etapar,StdErr=estimates.lpcm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.lpcm$loglik, digits=1),
"<br />Number of iterations:",estimates.lpcm$iter,"<br />Number of parameters:",estimates.lpcm$npar))
# keep results in current workspace
.GlobalEnv$estimates.lpcm <- estimates.lpcm
})
