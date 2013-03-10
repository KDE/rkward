#' Replace checkbox XML objects with JavaScript code
#' 
#' This function is a basically shortcut for \code{\link[rkwarddev:ite]{ite}} with some assumptions.
#' It's thought to be used when a checkbox should turn an option of an R function to a specified value,
#' by default \code{TRUE} or \code{FALSE} (hence the name, abbreviated "true or false").
#' The same result can be obtained with \code{ite}, but for most common cases \code{tf} is much quicker.
#' 
#' @param cbox An obkect of class \code{XiMpLe.node} containing a \code{<checkbox>} node, as generated
#'		by \code{\link[rkwarddev:rk.XML.cbox]{rk.XML.cbox}}.
#' @param true Logical or character, the value the option should get. E.g., if \code{true=TRUE} then the option will be
#'		set to \code{TRUE} if the box is checked, or in case \code{not=TRUE}, if the box is not checked.
#' @param not Logical, inverses the checked status of the checkbox. In other words, set this to \code{TRUE}
#'		if you want the option to be set if the box is not checked.
#' @param ifelse Logical, whether the the options should be set anyway. By default, the option will only
#'		be set in one condition. If \code{ifelse=TRUE}, it will get the inverse value in case of the alternative
#'		condition, e.g. it will be set to either \code{not=TRUE} or \code{not=FALSE} if the box is checked or unchecked.
#' @param false Logical or character, the value the option should, only used get if \code{ifelse=TRUE} as well.
#'		E.g., if \code{false=FALSE} then the option will be set to \code{FALSE} if the box is not checked,
#'		or in case \code{not=TRUE}, if the box is checked.
#' @param opt A character string, naming the R option o be set. If \code{NULL}, the XML ID of the checkbox node
#'		will be used.
#' @param prefix A character string, what should be pasted before the actual option string. Default is a
#'		comma and a newline.
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining the indentation string to use. This refers to the genrated R code,
#' 	not the JavaScript code. Indentation is added after the prefix and before the option string.
#' @return An object of class \code{rk.JS.ite}.
#' @export
#' @seealso \code{\link[rkwarddev:ite]{ite}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="foo_option")
#' tf(cbox1)

tf <- function(cbox, true=TRUE, not=FALSE, ifelse=FALSE, false=FALSE, opt=NULL, prefix=",\n", level=3, indent.by="\t"){

	# check if we're given a checkbox, alright...
	if(inherits(cbox, "XiMpLe.node")){
		node.name <- cbox@name
		if(!identical(node.name, "checkbox")){
			stop(simpleError(paste0("Invalid XML node, expected 'checkbox' and got: ", node.name)))
		} else {}
	} else {
		stop(simpleError("'cbox' must be of class XiMpLe.node!"))
	}

	if(is.null(opt)){
		opt.name <- id(cbox, js=FALSE)
	} else {
		opt.name <- opt
	}

	full.prefix <- paste0(prefix, indent(level=level, by=indent.by))

	# check for negation
	inverse <- ifelse(not, "!", "")
	result <- ite(id(inverse, cbox),
		echo(paste0(full.prefix, opt.name, "=", true)),
		ifelse(ifelse, echo(paste0(full.prefix, opt.name, "=", false)), "")
		)
	return(result)
}
