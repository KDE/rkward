# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

#' Render a preview of the given R markdown file
#'
#' This function is mostly targetted at providing a preview inside a script editor window. For other purposes
#' it will generally make more sense to call \link{rmarkdown::render}, directly.
#'
#' @param infile The input Rmd file, specified as a character string.
#' @param outdir The directory for output. The directory is not cleared, but existing files inside it may be overwritten without notice!
#' @param ... Additional parameters to pass to \link{rmarkdown::render}. Notably this may include \code{utput_format="html_document"} or
#'            \code{output_format="pdf_document"}.
#'
#' @rdname rk.render.markdown.preview
#' @export
"rk.render.markdown.preview" <- function(infile, outdir, ...) {
	.check.for.software <- function(command, message) {
		output <- ''
		for (i in 1:length(command)) {
			if (!nzchar(Sys.which(command[i]))) output <- paste0(output, '<h2>', .rk.i18n("Missing installed software"), '</h2><p>', message[i], '</p>\n')
		}
		output
	}
	require(rmarkdown)
	res <- try({
		rmarkdown::render(infile, output_dir=outdir, quiet=TRUE, ...)
	})
	if (inherits(res, 'try-error')) {
		msg <- attr(res, 'condition')$message
		out <- paste0('<h1>', .rk.i18n("Rendering the preview failed"), '</h1>')
		if (length(grep('pandoc', msg))) {
			out <- paste0(out, .check.for.software('pandoc', .rk.i18n(
			    "The software <tt>pandoc</tt>, required to rendering R markdown files, is not installed, or not in the system path of the running R session. You will need to install pandoc from <a href=\"https://pandoc.org/\">https://pandoc.org/</a>.</br>If it is installed, but cannot be found, try adding it to the system path of the running R session at <a href=\"rkward://settings/rbackend\">Settings->Configure RKward->R-backend</a>.")))
		}
		if (length(grep('pdflatex', msg))) {
			out <- paste0(out, .check.for.software('pdflatex', .rk.i18n("The software <tt>pdflatex</tt> is required for rendering PDF previews. The easiest way to install it is by running <tt>install.packages(\"tinytex\"); library(\"tinytex\"); install_tinytex()</tt>")))
		}
		rk.show.html(content=out)
		stop(.rk.i18n("Rendering the preview failed")) # make sure, the status display sees this as an error
	} else {
		if (endsWith(toupper(res), '.PDF')) {
			rk.show.pdf(res)
		} else if (endsWith(toupper(res), '.HTML')) {
			rk.show.html(res)
		} else {
			rk.show.html(content=paste0(.rk.i18n("<h1>Unsupported format</h1><p>The preview cannot be shown, here, because the output format is neither HTML, nor PDF. You can try opening it in an external application, using the link, below, or you can change the preview mode to 'R Markdown (HTML)'.</p>"), '<p><a href=\"', res, '\">', res, '</a></p>'))
		}
	}
}
