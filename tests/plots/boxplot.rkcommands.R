local({
## Prepare
## Compute
## Print result
data_list <- rk.list (women[["weight"]], women[["height"]])		#convert single sample variables to list
rk.header ("Boxplot", list ("Variable(s)", paste (names (data_list), collapse=", ")))
rk.graph.on()
try (boxplot (data_list, notch = FALSE, outline = TRUE, horizontal = FALSE)) #actuall boxplot function
	try (points(sapply(data_list,mean,na.rm = TRUE),pch=15, cex = 1.00, col="blue")) #calculates the mean for all data and adds a point at the corresponding position
rk.graph.off ()
})
