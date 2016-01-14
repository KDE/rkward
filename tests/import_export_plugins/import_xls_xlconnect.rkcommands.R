local({
## Prepare
require (XLConnect)
## Compute
data <- readWorksheetFromFile ("/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xlsx", sheet=1, startRow=0, startCol=0, endRow=7, endCol=0)

.GlobalEnv$my.xls.data <- data		# assign to globalenv()
## Print result
rk.header ("Import SPSS data", parameters=list("File name"="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xlsx",
	"Object to save to"="my.xls.data"))
})
local({
## Prepare
require (XLConnect)
## Compute
data <- readWorksheetFromFile ("/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls", sheet=1, region="A6:B9", header=FALSE)

.GlobalEnv$my.xlsx.data <- data		# assign to globalenv()
## Print result
rk.header ("Import SPSS data", parameters=list("File name"="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls",
	"Object to save to"="my.xlsx.data"))
})
