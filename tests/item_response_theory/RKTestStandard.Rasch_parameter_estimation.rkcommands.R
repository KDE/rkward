local({
## Prepare
  require(ltm)
## Compute
estimates.rasch <<- rasch(LSAT)
## Print result
rk.header ("Rasch parameter estimation")
rk.print (estimates.rasch$coefficients)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_rasch", settings="constraint.available=\nghk_rasch.real=21.00\nirtparam.state=TRUE\niterqn_rasch.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nstartval.string=NULL\nverbose.state=\nx.available=LSAT", label="Run again")
.rk.make.hr()
