local({
## Prepare
  require(eRm)
## Compute
estimates.lltm <<- LLTM(lltmdat1, mpoints=2)
## Print result
rk.header ("LLTM  parameter estimation")
rk.print (estimates.lltm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lltm", settings="design.string=auto\netastart.string=NULL\ngroups.string=1\nmpoints.real=2.00\nstderr.state=se\nsumnull.state=sum0\nx.available=lltmdat1", label="Run again")
.rk.make.hr()
