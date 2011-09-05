#' Create JavaScript outline from RKWard plugin XML
#'
#' @param require A character vector with names of R packages that the dialog depends on.
#' @param variables A character string to be included to read in all needed variables from the dialog.
#'		Refer to \code{\link{rk.JS.scan}} for a function to create this from an existing plugin XML file. 
#' @param results.header A character string to headline the printed results.
#' @param indent.by A character string defining how indentation should be done.
#' @return A character string.
#' @export

rk.JS.doc <- function(require=c(), variables=NULL, results.header=NULL, indent.by="\t"){

	js.require <- unlist(sapply(require, function(this.req){
			paste(indent(2, by=indent.by), "echo(\"require(", this.req, ")\\n\");", sep="")
		}))
	js.preprocess <- paste("function preprocess(){\n", indent(2, by=indent.by), "// add requirements etc. here.\n",
		paste(js.require, collapse="\n"), "\n}", sep="")

	js.calculate <- paste("function calculate(){\n",
			indent(2, by=indent.by), "// read in variables from dialog\n",
			paste(variables, collapse=""), "\n",
			indent(2, by=indent.by), "// create the R code to be evaluated here\n}", sep="")
		
	js.printout <- paste("function printout(){\n",
				indent(2, by=indent.by), "// printout the results\n",
				indent(2, by=indent.by), "echo(\"rk.header(\\\"", results.header,"\\\", level=2)\\n\");\n",
				indent(2, by=indent.by), "echo(\"rk.print(\\\"\\\")\\n\");\n}", sep="")

	JS.doc <- paste(js.preprocess, js.calculate, js.printout, sep="\n\n")

	return(JS.doc)
}
