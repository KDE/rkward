local({
## Compute
write.table ( x =  women , file =  "data" , append =  FALSE  ,quote = TRUE ,  sep = '\t' , eol = "\n" , na = "NA" ,  dec = '.' , row.names =  FALSE ,  col.names =  TRUE , qmethod=  'escape' ) 
## Print result
rk.header("Write as table", parameters=list("File", "data",
	"Data", "women"))
})
