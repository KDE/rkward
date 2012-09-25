local({
## Print result
groups <- rk.list (warpbreaks[["tension"]], warpbreaks[["wool"]])
data_list <- split (warpbreaks[["breaks"]], groups)		#split sample by grouping variables
rk.header ("Boxplot", list ("Outcome variable", rk.get.description (warpbreaks[["breaks"]]), "Grouping variable(s)", paste (names (groups), collapse=", ")))
rk.graph.on()
try ({
	boxplot (data_list, notch = FALSE, outline = FALSE, horizontal = TRUE) #actual boxplot function
})
rk.graph.off ()
})
local({
## Print result
groups <- rk.list (warpbreaks[["tension"]], warpbreaks[["wool"]])
data_list <- split (warpbreaks[["breaks"]], groups)		#split sample by grouping variables
# adjust width and position of boxes to achieve dodging
dodge_size <- nlevels (interaction (warpbreaks[["tension"]]))
box_width <- 0.80 / dodge_size
box_positions <- (rep (1:(length (data_list) / dodge_size), each=dodge_size) + (1:dodge_size)*(box_width))
rk.header ("Boxplot", list ("Outcome variable", rk.get.description (warpbreaks[["breaks"]]), "Grouping variable(s)", paste (names (groups), collapse=", ")))
rk.graph.on()
try ({
	boxplot (data_list, boxwex=box_width, at=box_positions, xlim=c(min(box_positions)-box_width, max(box_positions)+box_width), notch = FALSE, outline = FALSE, horizontal = TRUE) #actual boxplot function
})
rk.graph.off ()
})
