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
#'		pasted as-is, after \code{results.header} has been evaluated. Ignored if \code{doPrintout} is set.
#' @param doPrintout A character string to be included in the \code{doPrintout()} function. This string will be
#'		pasted as-is. You don't need to define a \code{preview()} function, as this will be added automatically. Use \code{ite("full", ...)} style JavaScript code to include headers etc.
#' @param indent.by A character string defining how indentation should be done.
#' @return A character string.
#' @seealso \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'		\code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:ite]{ite}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.JS.doc <- function(require=c(), variables=NULL, results.header=NULL,
	preprocess=NULL, calculate=NULL, printout=NULL, doPrintout=NULL, indent.by="\t"){

	js.require <- unlist(sapply(require, function(this.req){
			paste(indent(2, by=indent.by), "echo(\"require(", this.req, ")\\n\");\n", sep="")
		}))
	js.preprocess <- paste("function preprocess(){\n",
		indent(2, by=indent.by), "// add requirements etc. here\n",
		paste(js.require, collapse=""),
		ifelse(is.null(preprocess), "", paste(preprocess, "\n", sep="")),
		"}", sep="")

	js.calculate <- paste("function calculate(){\n",
			indent(2, by=indent.by), "// read in variables from dialog\n",
			ifelse(is.null(variables), "\n", paste(paste(variables, collapse=""), "\n", sep="")),
			indent(2, by=indent.by), "// put the R code to be evaluated here\n",
			ifelse(is.null(calculate), "", paste(calculate, "\n", sep="")),
			"}", sep="")
		
	js.printout <- paste("function printout(){\n",
				if(is.null(doPrintout)){paste(
					indent(2, by=indent.by), "// printout the results\n",
					indent(2, by=indent.by), "echo(\"rk.header(\\\"", results.header,"\\\", level=1)\\n\");\n",
					ifelse(is.null(printout), paste(indent(2, by=indent.by), "echo(\"rk.print(\\\"\\\")\\n\");", sep=""), printout),
					sep="")
				} else {paste(
					indent(2, by=indent.by), "// all the real work is moved to a custom defined function doPrintout() below\n",
					indent(2, by=indent.by), "// true in this case means: We want all the headers that should be printed in the output:\n",
					indent(2, by=indent.by), "doPrintout(true);",
					sep="")
				}, "\n}", sep="")

	# this part will create preview() and doPrintout(full), if needed
	if(is.null(doPrintout)){
		js.doPrintout <- ""
	} else {
		js.doPrintout <- paste("function preview(){\n",
					indent(2, by=indent.by), "preprocess();\n",
					indent(2, by=indent.by), "calculate();\n",
					indent(2, by=indent.by), "doPrintout(false);\n}\n\n",
					"function doPrintout(full){\n", doPrintout, "\n}",
				sep="")
	}

	JS.doc <- paste(js.preprocess, js.calculate, js.printout, js.doPrintout, sep="\n\n")

	return(JS.doc)
}
