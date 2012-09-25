local({
## Print result
data_list <- rk.list (women[["weight"]], women[["height"]])		#convert single sample variables to list
rk.header ("Boxplot", list ("Variable(s)", paste (names (data_list), collapse=", ")))
rk.graph.on()
try ({
	boxplot (data_list, notch = FALSE, outline = TRUE, horizontal = FALSE) #actual boxplot function
	points(sapply(data_list, mean, na.rm=TRUE), pch=15, cex = 1.00, col="blue") #calculates the mean for all data and adds a point at the corresponding position
})
rk.graph.off ()
})
local({
## Print result
data_list <- rk.list (women[["weight"]], women[["height"]])		#convert single sample variables to list
rk.header ("Boxplot", list ("Variable(s)", paste (names (data_list), collapse=", ")))
rk.graph.on()
try ({
	boxplot (data_list, notch = FALSE, outline = TRUE, horizontal = FALSE) #actual boxplot function
	geo_mean <- function (x) {prod(na.omit(x))^(1/length(na.omit(x)))}	#Calculate geometric mean
	points(sapply(data_list, geo_mean), pch=15, cex = 1.00, col="blue") #calculates the mean for all data and adds a point at the corresponding position
	sd_low <- (sapply(data_list, mean, na.rm = TRUE)) - (sapply(data_list,sd,na.rm = TRUE))
	sd_high <- (sapply(data_list, mean, na.rm = TRUE)) + (sapply(data_list,sd,na.rm = TRUE))
	points(sd_low, pch=3, cex = 1.00, col="blue")
	points(sd_high, pch=3, cex = 1.00, col="blue")
})
rk.graph.off ()
})
