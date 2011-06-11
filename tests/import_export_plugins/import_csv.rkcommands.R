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
.rk.rerun.plugin.link(plugin="rkward::import_csv", settings="allow_escapes.state=\nblanklinesskip.state=TRUE\ncheckname.state=TRUE\ncolclass.string=\ncolname.string=\ndec.string='.'\ndoedit.state=0\nfile.selection=women.csv\nflush.state=\nna.text=NA\nname.objectname=women\nname.parent=.GlobalEnv\nnomrow.text=1\nnrows.text=-1\nquick.string=csv\nquote.string='\\\\\\\"'\nrowname.string=rowcol\nsep.string=','\nskip.text=0\nstrings_as_factors.string=\nstripwhite.state=FALSE", label="Run again")
.rk.make.hr()
