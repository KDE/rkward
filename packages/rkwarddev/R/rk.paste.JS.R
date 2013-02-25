#' Paste JavaScript objects and character strings
#'
#' @note To get a list of the implemented modifiers in this package, call \code{rkwarddev:::all.valid.modifiers}.
#'
#' @param ... Objects of class \code{rk.JS.ite}, \code{rk.JS.arr}, \code{rk.JS.opt} or character.
#'		Another special case is XiMpLe nodes created by \code{rk.comment()}, which will be turned
#'		into JavaScript comments (i.e., lines starting with "//").
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining the indentation string to use.
#' @param funct For \code{rk.JS.arr} and \code{rk.JS.opt} objects only: Character string, name of the R function
#' 	to be called to combine the options, e.g. "list" for \code{list()}, or "c" for \code{c()}.
#' @param array For \code{rk.JS.opt} objects only: Logical, whether the options should be collected
#'		in an array or a concatenated character string.
#' @param var.prefix For \code{rk.JS.var} objects only: A character string. will be used as a prefix
#'		for the JS variable names.
#' @param modifiers For \code{rk.JS.var} objects only: A character vector with modifiers you'd like to apply the XML node's property.
#' @param default For \code{rk.JS.var} objects only: Logical, if \code{TRUE} the default value (no special modifier) of the node will
#'		also be defined. Does nothing if \code{modifiers=NULL}.
#' @param join For \code{rk.JS.var} objects only: A character string, useful for GUI elements which accept multiple objects
#'		(i.e., multi-varslots). If \code{join} is something other than \code{""}, these objects will be collapsed into one string
#'		when pasted, joined by this string.
#' @param getter For \code{rk.JS.var} objects only: A character string, naming the JavaScript function which should be used to get the
#'		values in the actual plugin. Depending on the XML element, \code{"getString"}, \code{"getBool"} or \code{"getList"} can be
#'		useful alternatives. For backwards compatibility, the default is set to \code{"getValue"}.
#' @param empty.e For \code{rk.JS.ite} objects only: Logical, if \code{TRUE} will force to add empty \code{else \{\}} brackets when
#'		there is no \code{else} statement defined, which is considered to enhance code readability by some.
#' @return A character string.
#' @include rk.JS.arr-class.R
#' @include rk.JS.ite-class.R
#' @include rk.JS.opt-class.R
#' @include rk.JS.var-class.R
#' @seealso
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:ite]{ite}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.paste.JS <- function(..., level=2, indent.by="\t", funct=NULL, array=NULL,
	var.prefix=NULL, modifiers=NULL, default=NULL, join=NULL, getter=NULL, empty.e=FALSE){
	stopifnot(level > 0)
	all.objects <- list(...)

	paste.results <- paste(sapply(all.objects, function(this.object){
		# if this is a plot options object, by default only paste the printout slot
		# and discard the rest
		this.object <- stripCont(this.object, get="printout")

		if(inherits(this.object, "rk.JS.ite")){
			# done by an internal function, to ease handling of recursions
			result <- paste.JS.ite(this.object, level=level, indent.by=indent.by, empty.e=empty.e)
		} else if(inherits(this.object, "rk.JS.arr")){
			# done by an internal function, to ease handling of recursions
			result <- paste.JS.array(this.object, level=level, indent.by=indent.by, funct=funct)
		} else if(inherits(this.object, "rk.JS.opt")){
			result <- paste.JS.options(this.object, level=level, indent.by=indent.by, array=array, funct=funct)
		} else if(inherits(this.object, "rk.JS.var")){
			result <- paste.JS.var(this.object, level=level, indent.by=indent.by, JS.prefix=var.prefix,
				modifiers=modifiers, default=default, join=join, getter=getter)
		} else if(inherits(this.object, "rk.JS.echo")){
			result <- slot(this.object, "value")
		} else if(is.XiMpLe.node(this.object)){
			if(identical(XMLName(this.object), "!--")){
				result <- paste(indent(level, by=indent.by),
					"// ",
					gsub("\n", paste("\n", indent(level, by=indent.by), "//", sep=""), XMLValue(XMLChildren(this.object)[[1]])), sep="")
			} else {
				stop(simpleError("XiMpLe.node objects are only valid if they are comments!"))
			}
		} else {
			# chop off beginning indent strings, otherwiese they ruin the code layout
			this.object <- gsub(paste("^", indent.by, "*", sep=""), "", this.object)
			result <- paste(indent(level, by=indent.by), this.object, sep="")
		}
		return(result)
	}), collapse="\n")

	return(paste.results)
}
