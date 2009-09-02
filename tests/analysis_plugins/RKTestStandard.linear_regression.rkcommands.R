local({
## Prepare
## Compute
results <- summary.lm (lm (warpbreaks[["breaks"]] ~ warpbreaks[["tension"]] + warpbreaks[["wool"]]))
## Print result
rk.header ("Linear Regression")
rk.print(results)
})
.rk.rerun.plugin.link(plugin="rkward::linear_regression", settings="intercept.state=1\nx.available=warpbreaks[[\\\"tension\\\"]]\\nwarpbreaks[[\\\"wool\\\"]]\ny.available=warpbreaks[[\\\"breaks\\\"]]", label="Run again")
.rk.make.hr()
