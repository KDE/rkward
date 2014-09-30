local({
## Prepare
require (foreign)

# helper function to convert all strings to the current encoding
iconv.recursive <- function (x, from) {
	attribs <- attributes (x);
	if (is.character (x)) {
		x <- iconv (x, from=from, to="", sub="")
	} else if (is.list (x)) {
		x <- lapply (x, function (sub) iconv.recursive (sub, from))
	}
	# convert factor levels and all other attributes
	attributes (x) <- lapply (attribs, function (sub) iconv.recursive (sub, from))
	x
}
## Compute
data <- read.dta ("import_export_plugins_testfile.dta", convert.dates=TRUE, convert.factors=TRUE, missing.type=FALSE, convert.underscore=FALSE)

# convert all strings to the current encoding
data <- iconv.recursive (data, from="ISO8859-1")

# set variable labels for use in RKWard
labels <- attr (data, "var.labels")
if (!is.null (labels)) {
        for (i in 1:length (labels)) {
                col <- make.names (attr (data, "names")[i] )
                if (!is.null (col)) {
                        rk.set.label (data[[col]], labels[i])
                }
        }
}

.GlobalEnv$my.stata.data <- data		# assign to globalenv()
## Print result
rk.header("Import Stata File", parameters=list("File", "import_export_plugins_testfile.dta",
	"Imported to", "my.stata.data"))
})
