local({
## Compute

	# some options can't be changed with write.csv() and are set to these values:
	# append=FALSE, sep=",", dec=".", col.names=NA, qmethod="double"
	write.csv(
		x=women,
		file="PATH/data",
		fileEncoding=""
	)

## Print result
rk.header ("Export Table / CSV files", parameters=list("File"="PATH/data",
	"Data"="women"))
})
