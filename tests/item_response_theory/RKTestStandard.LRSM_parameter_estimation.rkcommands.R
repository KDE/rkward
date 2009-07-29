local({
## Prepare
  require(eRm)
## Compute
estimates.lrsm <<- LRSM(lrsmdat, mpoints=2, se=FALSE, sum0=FALSE)
## Print result
rk.header ("LRSM  parameter estimation")
rk.print (estimates.lrsm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_lrsm", settings="design.string=auto\netastart.string=NULL\ngroups.string=1\nmpoints.real=2.000000\nstderr.state=\nsumnull.state=\nx.available=lrsmdat", label="Run again")
.rk.make.hr()
