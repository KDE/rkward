local({
## Prepare
require(eRm)
## Compute
estimates.rsm <- RSM(rsmdat)
## Print result
rk.header ("RSM  parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.rsm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print(t(rbind(Eta=estimates.rsm$etapar,StdErr=estimates.rsm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.rsm$loglik, digits=1),
"<br />Number of iterations:",estimates.rsm$iter,"<br />Number of parameters:",estimates.rsm$npar))
# keep results in current workspace
.GlobalEnv$estimates.rsm <- estimates.rsm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_rsm", settings="design.string=auto\netastart.string=NULL\nsave_name.active=1\nsave_name.objectname=estimates.rsm\nsave_name.parent=.GlobalEnv\nstderr.state=se\nsumnull.state=sum0\nx.available=rsmdat", label="Run again")
.rk.make.hr()
