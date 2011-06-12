local({
## Prepare
require (foreign)
## Compute
data <- read.spss ("import_export_plugins_testfile.sav", to.data.frame=TRUE, max.value.labels=1)

# set variable labels for use in RKWard
labels <- attr (data, "variable.labels");
if (!is.null (labels)) {
	for (i in 1:length (labels)) {
		col <- make.names (names (labels[i]))
		if (!is.null (col)) {
			rk.set.label (data[[col]], labels[i])
		}
	}
}

.GlobalEnv$my.spss.data <- data		# assign to globalenv()
## Print result
rk.header("Import SPSS data", parameters=list("File", "import_export_plugins_testfile.sav",
	"Import as", "my.spss.data"))
})
