#' Create XML node "formula" for RKWard plugins
#'
#' @param fixed The \code{id} of the varslot holding the selected fixed factors.
#' @param dependent The \code{id} of the varslot holding the selected dependent variable.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.formula <- rk.XML.formula(fixed="vrs.one", dependent="vsr.two")
#' cat(pasteXMLNode(test.formula, shine=1))

rk.XML.formula <- function(fixed, dependent){
	node <- new("XiMpLe.node",
			name="formula",
			attributes=list("fixed_factors"=fixed, dependent=dependent)
		)

	return(node)
}
