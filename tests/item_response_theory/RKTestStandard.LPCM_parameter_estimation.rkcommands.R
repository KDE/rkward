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
estimates.lpcm <<- estimates.lpcm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lpcm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\ngroup_vec.available=G\ngroups.string=contrasts\nmpoints.real=2.00\nsave_name.selection=estimates.lpcm\nstderr.state=se\nsumnull.state=sum0\nx.available=lpcmdat", label="Run again")
.rk.make.hr()
