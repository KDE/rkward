local({
## Prepare
## Compute
write.table ( x =  women , file =  "data" , append =  FALSE  ,quote = TRUE ,  sep = '\t' , eol = "\n" , na = "NA" ,  dec = '.' , row.names =  FALSE ,  col.names =  TRUE , qmethod=  'escape' ) 
## Print result
rk.header("Write as table", parameters=list("File", "data",
	"Data", "women"))
})
.rk.rerun.plugin.link(plugin="rkward::save_table", settings="append.state=FALSE\ncolumns.string=TRUE\ndata.available=women\ndec.string='.'\neol.text=\\\\n\nfile.selection=data\nna.text=NA\nqmethod.string='escape'\nquote.state=TRUE\nrows.string=FALSE\nsep.string='\\\\t'", label="Run again")
.rk.make.hr()
