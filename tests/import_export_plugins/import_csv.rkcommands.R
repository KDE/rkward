local({
## Prepare
## Compute
imported <- read.csv (file="women.csv", row.names=1,  na.strings = "NA", nrows = -1, skip = 0, check.names = TRUE, strip.white = FALSE, blank.lines.skip = TRUE)

# copy from the local environment to globalenv()
.GlobalEnv$women <- imported
## Print result
rk.header("Import text / csv data", parameters=list("File", "women.csv",
	"Import as", "women"))
})
