local({
## Prepare
  require(eRm)
## Compute
estimates.lpcm <- LPCM(lpcmdat, mpoints=2, groupvec=G)
## Print result
rk.header ("LPCM  parameter estimation")
rk.print (estimates.lpcm)
# keep results in current workspace
estimates.lpcm <<- estimates.lpcm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lpcm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\ngroup_vec.available=G\ngroups.string=contrasts\nmpoints.real=2.00\nsave_name.selection=estimates.lpcm\nstderr.state=se\nsumnull.state=sum0\nx.available=lpcmdat", label="Run again")
.rk.make.hr()
