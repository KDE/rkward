local({
## Berechne

	# some options can't be changed with write.csv() and are set to these values:
	# append=FALSE, sep=",", dec=".", col.names=NA, qmethod="double"
	write.csv(
		x=women,
		file="data",
		fileEncoding=""
	)

## Drucke Ergebnisse
rk.header ("Export Table / CSV files", parameters=list("File"="data",
	"Data"="women"))
})