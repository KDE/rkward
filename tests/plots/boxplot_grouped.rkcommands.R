local({
## Prepare
## Compute
## Print result
groups <- rk.list (warpbreaks[["tension"]], warpbreaks[["wool"]])
data_list <- split (warpbreaks[["breaks"]], groups)		#split sample by grouping variables
rk.header ("Boxplot", list ("Outcome variable", rk.get.description (warpbreaks[["breaks"]]), "Grouping variable(s)", paste (names (groups), collapse=", ")))
rk.graph.on()
try (boxplot (data_list, notch = FALSE, outline = FALSE, horizontal = TRUE)) #actuall boxplot function
rk.graph.off ()
})
