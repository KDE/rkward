local({
## Prepare
require(ltm)
## Compute
personfit.res <- person.fit(estimates.rasch)
## Print result
rk.header ("Person-fit statistics (estimates.rasch)")
rk.print ("Call:")
rk.print.literal (deparse(personfit.res$call, width.cutoff=500))
rk.header ("Response patterns, person-fit statistics (L0, Lz) and probabilities for each response pattern (Pr):", level=4)
rk.print(cbind(format(personfit.res$resp.patterns, nsmall=0), round(personfit.res$Tobs, digits=3), "Pr (&lt;Lz)"=round(c(personfit.res$p.values), digits=3)))
})
