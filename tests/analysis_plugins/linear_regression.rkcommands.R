local({
## Compute
results <- summary.lm (lm (warpbreaks[["breaks"]] ~ warpbreaks[["tension"]] + warpbreaks[["wool"]]))
## Print result
rk.header ("Linear Regression")
rk.print(results)
})
