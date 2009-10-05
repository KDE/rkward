local({
## Prepare
require (foreign)
## Compute
data <- read.spss ("../import_export_plugins_testfile.sav", to.data.frame=TRUE, max.value.labels=1)

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

my.spss.data <<- data		# assign to globalenv()
## Print result
rk.header("Import SPSS data", parameters=list("File", "../import_export_plugins_testfile.sav",
	"Import as", "my.spss.data"))
})
.rk.rerun.plugin.link(plugin="rkward::import_spss", settings="convert_var_labels.state=1\ndata_frame.state=1\ndo_locale_conversion.state=0\ndoedit.state=0\nfile.selection=../import_export_plugins_testfile.sav\nlabels_limit.real=1.00\nsaveto.selection=my.spss.data\ntrim_labels.state=0\nuse_labels.state=1", label="Run again")
.rk.make.hr()
