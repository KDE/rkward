local({
## Prepare
  require(ltm)
## Compute
personfit.res <- person.fit(estimates.rasch)
## Print result
rk.header ("Person-fit statistics (estimates.rasch)")
rk.print (personfit.res)
})
.rk.rerun.plugin.link(plugin="rkward::ltm_person_fit", settings="rad_hypot.string=less\nrad_pvalue.string=normal\nrad_resppat.string=observed\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
