local({
## Prepare
  require(eRm)
## Compute
estimates.lpcm <<- LPCM(lpcmdat, mpoints=2, groupvec=G)
## Print result
rk.header ("LPCM  parameter estimation")
rk.print (estimates.lpcm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lpcm", settings="design.string=auto\netastart.string=NULL\ngroup_vec.available=G\ngroups.string=contrasts\nmpoints.real=2.000000\nstderr.state=se\nsumnull.state=sum0\nx.available=lpcmdat", label="Run again")
.rk.make.hr()
