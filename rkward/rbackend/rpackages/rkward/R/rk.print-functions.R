# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Print objects and results to output
#' 
#' Various utilty functions which can be used to print or export R objects to
#' the (html) output file. The output file can be accessed from Windows -> Show
#' Output. Basically, these functions along with the ones described in
#' \code{\link{rk.get.label}}, \code{\link{rk.get.tempfile.name}}, and
#' \code{\link{rk.graph.on}} can be used to create a HTML report.
#' 
#' \code{rk.print} prints/exports the given object to the output (html) file
#' using the \code{\link[R2HTML:HTML]{HTML}} function. This requires the \code{R2HTML}
#' package. Additional arguments in \code{...} are passed on to
#' \code{\link[R2HTML:HTML]{HTML}}. For some types of objects (e.g. "htmlwidgets" from
#' the "htmlwidgets" package) additional packages may be required.
#' 
#' \code{rk.print.literal} prints/exports the given object using a
#' \code{paste(x, collapse="\n")} construct to the output (html) file.
#' 
#' \code{rk.print.code} applies syntax highlighting to the given code string,
#' and writes it to the output (html) file.
#' 
#' \code{rk.header} prints a header / caption, possibly with parameters, to the
#' output file. See example.
#' 
#' \code{rk.results} is similar to \code{rk.print} but prints in a more
#' tabulated fashion. This has been implemented only for certain types of
#' \code{x}: tables, lists (or data.frames), and vectors. See example.
#' 
#' \code{rk.describe.alternatives} describes the alternative (H1) hypothesis of
#' a \code{htest}. This is similar to \code{stats:::print.htext} and makes
#' sense only when \code{x$alternatives} exists.
#' 
#' @aliases rk.print rk.print.code rk.print.literal rk.header rk.results
#'   rk.describe.alternative
#' @param x any R object to be printed/exported. A suitable list in case of
#'   \code{rk.describe.alternative}.
#' @param code a character vector (single string) of R code
#' @param title a string, used as a header for the html output
#' @param level an integer, header level. For example, \code{level=2} creates
#'   the header with \code{<h2></h>} tag.
#' @param parameters a list, preferably named, giving a list of "parameters" to
#'   be printed to the output
#' @param toc If \code{NULL}, the default, \code{rk.header()} will automatically
#'   add headers h1 to h4 to the TOC menu of the output document. \code{TRUE} will always
#'   add the header, and \code{FALSE} will suppress it.
#' @param titles a character vector, giving the column headers for a html
#'   table.
#' @param print.rownames controls printing of rownames. TRUE to force printing,
#'   FALSE to suppress printing, omitted (default) to print rownames, unless
#'   they are plain row numbers.
#' @param ... Additional arguments to be passed on to \code{\link[R2HTML:HTML]{HTML}}.
#' @return \code{rk.describe.alternatives} returns a string while all other
#'   functions return \code{NULL}, invisibly.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link[R2HTML:HTML]{HTML}}, \code{\link{rk.get.output.html.file}},
#'   \code{\link{rk.get.description}}, \code{\link{rk.call.plugin}},
#'   \url{rkward://page/rkward_output}
#' @keywords utilities
#' @rdname rk.results
#' @importFrom grDevices dev.cur
#' @export
#' @examples
#' \dontrun{
#' # code is only run when the R2HTML package can be loaded
#' if(require("R2HTML", quietly = TRUE)){
#' 
#'   ## see the output: Windows->Show Output
#'   ## stolen from the two-sample t-test plugin ;)
#'   local({
#'   x1 <- rnorm (100)
#'   x2 <- rnorm (100, 2)
#'   nm <- rk.get.description (x1,x2)
#' 
#'   result <- t.test (x1, x2, alternative="less")
#'   rk.print.code ("result <- t.test (x1, x2, alternative=\"less\")")
#' 
#'   rk.header (result$method,
#'     parameters=list ("Comparing", paste (nm[1], "against", nm[2]),
#'     "H1", rk.describe.alternative (result),
#'     "Equal variances", "not assumed"))
#' 
#'   rk.print.literal ("Raw data (first few rows):")
#'   rk.print (head (cbind (x1,x2)), align = "left")
#' 
#'   rk.print.literal ("Test results:")
#'   rk.results (list (
#'     'Variable Name'=nm,
#'     'estimated mean'=result$estimate,
#'     'degrees of freedom'=result$parameter,
#'     t=result$statistic,
#'     p=result$p.value,
#'     'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
#'     'confidence interval of difference'=result$conf.int ))
#'   })
#' } else {}
#' }
"rk.print" <- function(x, ...) {
	if (inherits (x, "htmlwidget")) {
		requireNamespace ("htmlwidgets", quietly = TRUE)
		name <- deparse (substitute (x))
		filename <- rk.get.tempfile.name (name, ".html")
		dir <- rk.get.tempfile.name (name, "_data")
		htmlwidgets::saveWidget (x, filename, selfcontained=FALSE, libdir=dir)
		.rk.cat.output (paste0 ("<object width=\"100%\" height=\"100%\" data=\"file://", filename, "\" onload=\"this.style.height = this.contentWindow.document.body.scrollHeight + 'px';\"></object>"))
	} else if (inherits (x, "gvis")) {
		requireNamespace ("googleVis", quietly = TRUE)
		print (x, file=rk.get.output.html.file(), append=TRUE)
	} else {
		htmlfile <- rk.get.output.html.file()
		if(requireNamespace ("R2HTML", quietly = TRUE)) {
			R2HTML::HTML(x, file=htmlfile, ...)
		} else {
			.rk.cat.output("Please install package R2HTML to enable output!")
		}
	}
}

#' @export
#' @rdname rk.results
"rk.print.code" <- function(code) {
	.rk.cat.output(.rk.call("highlightRCode", as.character(code)))
}

#' @export
#' @rdname rk.results
"rk.header" <- function (title, parameters=list (), level=1, toc=NULL) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	# give header a name to be able to set anchors
	# it's just a time string down to the fraction of a second: yyyy-mm-dd HH:MM:SS.ssssss
	if (!isTRUE (options (".rk.suppress.toc")[[1]])) {	# disabled during plugin automated testing
		header.id <- format(Sys.time(), "%Y-%m-%d_%H:%M:%OS6")
		header.title <- format(Sys.time(), "%Y-%m-%d&nbsp;%H:%M:%S")
		# add 'id', 'name' and 'title' attributes to the header
		cat ("<h", level, "><a id=\"", header.id,"\" name=\"", header.id,"n\" title=\"", header.title,"\">", title, "</a></h", level, ">\n", sep="")
		# if 'toc' is true, also add a javascript function call to add this header to the TOC menu
		# the function addToTOC() will be defined in the document head
		# see rk.set.output.html.file() in rk.filename-functions.R
		if (isTRUE(toc) || (is.null(toc) && level <= 4)){
			cat("<script type=\"text/javascript\">\n\t<!--\n\t\taddToTOC('",header.id,"','",level,"');\n\t// -->\n</script>\n", sep="")
		}
	} else {
		cat ("<h", level, ">", title, "</h", level, ">\n", sep="")
	}

	if (length (parameters)) {
		# legacy handling: parameter=value used to be passed as parameter, value
		if (is.null (names (parameters))) {
			warning ("Unnamed parameter lists are deprecated in rk.header()")
			s <- seq.int (1, length (parameters), by=2)
			pnames <- as.character (parameters[s])
			parameters <- parameters[s+1]
		} else {
			pnames <- names (parameters)
		}

		cat ("<h", level + 1, ">Parameters</h", level + 1, ">\n<ul>", sep="")
		for (i in 1:length (parameters)) {
			cat ("<li>", pnames[i], ": ", parameters[[i]], "</li>\n", sep="")
		}
		cat ("</ul>\n")
	}
	if (level==1) cat (.rk.date ())
	cat ("<br />\n")
}

# Dummy to allow the rkwardtest package to override rk.header() behavior, easily
".rk.date" <- function () date ()

#' @export
#' @rdname rk.results
"rk.results" <- function (x, titles=NULL, print.rownames) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	# convert 2d tables to data.frames with values labelled
	if ((is.table(x) || is.matrix(x)) && (length(dim(x)) == 2)) {
		rows = dim(x)[1]
		cols = dim(x)[2]
		if (is.null(titles)) {
			titles <- names(dimnames(x))
		}
		rn <- dimnames(x)[[1]]
		if (!is.na (titles[1])) rn <- paste(titles[1], "=", rn)
		cn <- dimnames(x)[[2]]
		if (!is.na (titles[2])) cn <- paste(titles[2], "=", cn)
		titles <- c ("", cn)
		x <- data.frame (cbind (x), stringsAsFactors=FALSE)
		rownames (x) <- as.character (rn)
		if (missing (print.rownames)) print.rownames <- TRUE
	}

	if (is.list (x)) {	# or a data.frame
		if (is.data.frame (x)) {
			# by default, print rownames, unless they are just plain row numbering
			if (missing (print.rownames)) print.rownames <- !isTRUE (all.equal (rownames (x), as.character (1:dim(x)[1])))
			if (isTRUE (print.rownames)) {
				x <- cbind (rownames (x), x, stringsAsFactors=FALSE)
				names (x)[1] <- '';
			}
		}
		if (is.null (titles)) {
			titles <- names (x)
		}

		cat ("<table border=\"1\">\n<tr>")
		try ({	# if anything fails, make sure the "</table>" is still printed
			for (i in 1:length (x)) {
				cat ("<td>", titles[i], "</td>", sep="")
			}
			cat ("</tr>\n")
	
			if (is.data.frame (x)) {
				x <- as.data.frame (lapply (x, format), stringsAsFactors=FALSE)
				for (row in 1:dim (x)[1]) {
					cat ("<tr>")
					for (col in 1:dim (x)[2]) {
						cat ("<td>", x[row, col], "</td>", sep="")
					}
					cat ("</tr>\n")
				}
			} else {		# generic list
				cat ("<tr>")
				for (col in x) {
					col <- as.vector (col)
					cat ("<td>")
					for (row in 1:length (col)) {
						if (row != 1) cat ("\n<br/>")
						cat (col[row])
					}
					cat ("</td>")
				}
				cat ("</tr>\n")
			}
		})
		cat ("</table>\n")
	} else if (is.vector (x)) {
		cat ("<h3>", titles[1], ": ", sep="")
		cat (x)
		cat ("</h3>")
	} else {
		stop ("uninmplemented")
	}
}

#' @export
#' @rdname rk.results
"rk.print.literal" <- function (x) {
	cat ("<pre>", paste (x, collapse="\n"), "</pre>\n", sep="", file=rk.get.output.html.file(), append=TRUE);
}

# Describe the alternative (H1) of an htest.
# This code adapted from stats:::print.htest
#' @export
#' @rdname rk.results
"rk.describe.alternative" <- function (x) {
	res <- ""
	if (!is.null(x$alternative)) {
		if (!is.null(x$null.value)) {
 			if (length(x$null.value) == 1) {
 				alt.char <- switch(x$alternative, two.sided = "not equal to", less = "less than", greater = "greater than")
 				res <- paste ("true", names(x$null.value), "is", alt.char, x$null.value)
 			} else {
 				res <- paste (x$alternative, "\nnull values:\n", x$null.value)
 			}
		} else {
			res <-  (x$alternative)
		}
	}
	res
}

#' Capture output including system commands
#'
#' This feature records and / or redirects output similar to the \code{\link{sink}} and \code{\link{capture.output}}
#' commands provided with R. Contrary to these two, \code{rk.capture.output} can also capture output generated by external
#' programs running via \code{\link{system}} (on both Windows and Unix platforms). Further it allows to capture both
#' warning messages and regular output to the same channel. Output can be passed on for printing, or suppressed,
#' with the default being to print it.
#'
#' And arbitrary number of ouptut captures can be started and stopped, and these will form a push/pop stack.
#'
#' @param capture.messages logical, whether messages should be captured.
#' @param capture.output logical, whether output should be captured.
#' @param suppress.messages logical, if \code{FALSE} message are not sent on, neither to surrounding captures, nor as regular output.
#' @param suppress.output like \code{suppress.messages}, but for output.
#' @param allow.nesting if false, output / message is not sent to surrounding captures, but is sent to regular output (unless suppressed)
#'
#' \code{rk.capture.output} Starts capturing output.
#' 
#' \code{rk.end.capture.output} Stops capturing output, and returns the recorded output in a string.
#'
#' @export
#' @rdname rk.capture.output
"rk.capture.output" <- function (capture.messages = TRUE, capture.output = TRUE, suppress.messages = FALSE, suppress.output = FALSE, allow.nesting = TRUE) {
	.Call ("rk.capture.output", 1, isTRUE (capture.messages), isTRUE (capture.output), isTRUE (suppress.messages), isTRUE (suppress.output), isTRUE (allow.nesting), PACKAGE="(embedding)");
	invisible (NULL)
}

# what exactly does 'highlight' do?
#' @param highlight logical.
#' @export
#' @rdname rk.capture.output
"rk.end.capture.output" <- function (highlight = FALSE) {
	.Call ("rk.capture.output", if (isTRUE (highlight)) 2 else 3, FALSE, FALSE, FALSE, FALSE, FALSE, PACKAGE="(embedding)");
}
