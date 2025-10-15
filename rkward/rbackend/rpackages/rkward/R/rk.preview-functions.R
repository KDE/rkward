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

#' Evaluate the given input file, recording a transcript to an HTML output file (including on-screen plots) 
#'
#' @param infile The input R file, specified as a character string, or a connection (passed to \link{parse}).
#' @param outfile The output HTML file, specified as a character string. If this file exists, it will be overwritten, without further notice!
#' @param echo Include the source expressions in the output? (boolean)
#' @param env Environment of the evaluation. See details, below.
#' @param stop.on.error Whether to stop (TRUE) or continue (FALSE) on errors
#'
#' @details Contrary to \link{source}, some effort is made to avoid lasting side-effects to the workspace, however these cannot be ruled out,
#'          in all cases. Among other things, the following may lead to lasting effects:
#'
#'          \itemize{
#'              \item{Installing / updating / removing / loading packages}
#'              \item{Writing to the filesystem in any form}
#'              \item{Targetting pre-existing graphics devices with \code{dev.set()}, \code{dev.off()}, etc.}
#'              \item{Opening / closing windows in the RKWard workplace}
#'              \item{Exclusively assuming the R engine (e.g. shiny apps)}
#'              \item{Assignments outside the current scope (see also below)}
#'          }
#'
#'          The default argument value for \code{env} allows the evaluated script to access objects inside the \code{globalenv()}, but
#'          limits regular assignments (i.e. using \code{<-} rather than \code{<<-} or \code{assign()}) to a temporary local scope. Depending on the
#'          desired semantics, \code{new.env()} or \code{globalenv()} may be useful alternatives.
#'
#'          The idea of \code{rk.eval.as.preview} is to visualize what would happen when running the given code in the R console,
#'          interactively. Importantly, however, due to the evaluation inside a function, any error messages and backtraces will differ,
#'          and taskCallbacks will not run.
#'
#' @rdname rk.eval.as.preview
#' @export
rk.eval.as.preview <- function(infile, outfile, echo=TRUE, env=new.env(parent=globalenv()), stop.on.error=FALSE) {
	## init output file
	output <- rk.set.output.html.file(outfile, silent=TRUE)
	on.exit({
		rk.set.output.html.file(output, silent=TRUE)
	}, add=TRUE)
	suppressWarnings(try(rk.flush.output(ask=FALSE, style="preview", silent=TRUE)))

	# TODO: It may be better to split out this and other long literals into separate files
	.rk.cat.output(
"<script>
function expandPlots(expand) {
	let elements = [...document.querySelectorAll('details')];

	if (expand) {
		elements.map(item => item.setAttribute('open', 'true'));
	} else {
		elements.map(item => item.removeAttribute('open'));
	}
};

let plotelements = 0;
function registerPlot(element) {
	let index = ++plotelements;
	if (index == 1) {
		document.getElementById('plotbuttons').innerHTML=\'<div style=\"text-align:right\"><button onClick=\"expandPlots(true)\">Expand plots</button><button onClick=\"expandPlots(false)\">Collapse plots</button></div>\';
	}
	if (sessionStorage.getItem(window.location.pathname + 'plot' + index) == 'true') {
		element.setAttribute('open', 'true');
	}
	element.addEventListener('toggle', (event) => {
		sessionStorage.setItem(window.location.pathname + 'plot' + index, element.open ? 'true' : 'false')
	});
}
</script>
<span id='plotbuttons'></span>");

	## set up handling of generated graphics:
	devs <- list()
	prevdev <- NULL
	oldopts <- options() # while at it, save _all_ options. Script might change some, too
	options(device="RK") # just in case
	# avoid flicker (and _some_ wasted CPU cycles), by suppressing display of plot windows created, here
	oldsup <- .rk.suppress.RK.windows(TRUE)
	on.exit({
		.rk.suppress.RK.windows(oldsup)
	}, add=TRUE)

	# If a device already exists, let's open a new one to avoid touching it, unintentionally
	# We don't want that to show in the preview, however, which may or may not plot anything at all
	# NOTE: this does not help, if user script has unbalanced dev.off()-calls, of course
	if (length(dev.list()) > 0) {
		prevdev <- dev.cur()
		rk.without.plot.history(RK())
		devs[[as.character(dev.cur())]] <- RK.revision(dev.cur())
	}

	hook <- RK.addHook(
		after.create=function(devnum, ...) {
			.rk.cat.output("<div align=\"right\">", .rk.i18n("Plot window created"), "</div>\n");
			devs[[as.character(devnum)]] <<- RK.revision(devnum)
		},
		in.close=function(devnum, ...) {
			.rk.cat.output("<div align=\"right\">", .rk.i18n("Plot window closed"), "</div>\n");
			devs[[as.character(devnum)]] <<- NULL
		}
	)

	checkSavePlot <- function() {
		for (devnum in names(devs)) {
			currev <- RK.revision(as.numeric(devnum))
			if (devs[[devnum]] < currev) {
				cur <- dev.cur()
				.rk.cat.output("<div align=\"right\"><details><script>registerPlot(document.currentScript.parentElement);</script><summary>", .rk.i18n("Plot updated (click to show)"), "</summary><p>\n");
				#rk.graph.on(width=200, height=200, pointsize=6)
				rk.graph.on()
				out <- dev.cur()
				try({
					dev.set(as.numeric(devnum))
					dev.copy(which=out)
				})
				rk.graph.off()
				.rk.cat.output("</p></details></div>\n");
				dev.set(cur)
				devs[[devnum]] <<- currev
			}
		}
	}

	on.exit({
		RK.removeHook(hook)

		rk.without.plot.history({
			for (dev in names(devs)) {
				dev.off(as.numeric(dev))
			}

			if (!is.null(prevdev)) {
				dev.set(prevdev)
			}
		})

		options(oldopts)
	}, add=TRUE)

	## parse and evaluate
	# capture any parse errors
	exprs <- expression(NULL)
	rk.capture.output(suppress.messages=TRUE)
	res <- try({
		exprs <- parse(infile, keep.source=TRUE)
	})
	.rk.cat.output(rk.end.capture.output(TRUE))
	if(isTRUE(stop.on.error) && inherits(res, "try-error")) stop(res)

	# actually do it
	rk.without.plot.history({
		for (i in seq_len(length(exprs))) {
			if (isTRUE(echo)) {
				rk.print.code(as.character(attr(exprs, "srcref")[[i]]))
			}
			rk.capture.output(suppress.messages=TRUE, suppress.output=TRUE)
			res <- try({
				withAutoprint(exprs[[i]], evaluated=TRUE, echo=FALSE, local=env)
			})
			.rk.cat.output(rk.end.capture.output(TRUE))
			checkSavePlot()
			if(isTRUE(stop.on.error) && inherits(res, "try-error")) stop(res)
		}
	})

	# clean up is done via on.exit handlers, above
	invisible()
}
