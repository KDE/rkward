#' Create JavaScript outline from RKWard plugin XML
#' 
#' @note The JavaScript
#'
#' @param require A character vector with names of R packages that the dialog depends on.
#' @param variables A character string to be included to read in all needed variables from the dialog.
#'		Refer to \code{\link{rk.JS.scan}} for a function to create this from an existing plugin XML file. 
#' @param results.header A character string to headline the printed results.
#' @param preprocess A character string to be included in the \code{preprocess()} function. This string will be
#'		pasted as-is, after \code{require} has been evaluated.
#' @param calculate A character string to be included in the \code{calculate()} function. This string will be
#'		pasted as-is, after \code{variables} has been evaluated.
#' @param printout A character string to be included in the \code{printout()} function. This string will be
#'		pasted as-is, after \code{results.header} has been evaluated.
#' @param indent.by A character string defining how indentation should be done.
#' @return A character string.
#' @export

rk.JS.doc <- function(require=c(), variables=NULL, results.header=NULL,
	preprocess=NULL, calculate=NULL, printout=NULL, indent.by="\t"){

	js.require <- unlist(sapply(require, function(this.req){
			paste(indent(2, by=indent.by), "echo(\"require(", this.req, ")\\n\");", sep="")
		}))
	js.preprocess <- paste("function preprocess(){\n",
		indent(2, by=indent.by), "// add requirements etc. here.\n",
		paste(js.require, collapse="\n"), "\n",
		ifelse(is.null(preprocess), "", preprocess),
		"\n}", sep="")

	js.calculate <- paste("function calculate(){\n",
			indent(2, by=indent.by), "// read in variables from dialog\n",
			paste(variables, collapse=""), "\n",
			indent(2, by=indent.by), "// put the R code to be evaluated here.\n",
			ifelse(is.null(calculate), "", calculate),
			"\n}", sep="")
		
	js.printout <- paste("function printout(){\n",
				indent(2, by=indent.by), "// printout the results\n",
				indent(2, by=indent.by), "echo(\"rk.header(\\\"", results.header,"\\\", level=2)\\n\");\n",
				ifelse(is.null(printout), paste(indent(2, by=indent.by), "echo(\"rk.print(\\\"\\\")\\n\");", sep=""), printout),
				"\n}", sep="")

	JS.doc <- paste(js.preprocess, js.calculate, js.printout, sep="\n\n")

	return(JS.doc)
}
