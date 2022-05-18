local({
## Prepare
require(rio)
## Compute
data <- import("PATH/import_export_plugins_testfile.sav")
.GlobalEnv$my.rio.data <- data  # assign to globalenv()
## Print result
rk.header ("Generic data import", parameters=list("File name"="PATH/import_export_plugins_testfile.sav",
	"Object to save to"="my.rio.data"))
})
