#' Create XML empty node "stretch" for RKWard plugins
#'
#' @return An object of class \code{XiMpLe.node}.
#' @export

#<stretch />
rk.XML.stretch <- function(){
	return(new("XiMpLe.node", name="stretch"))
}
