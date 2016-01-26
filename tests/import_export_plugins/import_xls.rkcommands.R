local({
## Prepare
require (gdata)
## Compute
data <- read.xls ("/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls", sheet="1", header=TRUE, verbose=FALSE,  nrows=-1, skip=0, na.string="NA", strip.white = FALSE)
.GlobalEnv$my.xls.data <- data		# assign to globalenv()
## Print result
rk.header ("Import Microsoft EXCEL sheet", parameters=list("File name"="/home/thomas/develop/rkward/tests/import_export_plugins_testfile.xls",
	"Object to save to"="my.xls.data",
	"Name or number of sheet"="1",
	"Use first row as column names"="yes",
	"Number of rows to skip"="0",
	"Max number of rows to read (-1 for no limit)"="-1",
	"Character for missing values"="NA"))
})
