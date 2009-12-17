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
estimates.lrsm <<- estimates.lrsm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lrsm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\ngroups.string=1\nmpoints.real=2.00\nsave_name.selection=estimates.lrsm\nstderr.state=\nsumnull.state=\nx.available=lrsmdat", label="Run again")
.rk.make.hr()
