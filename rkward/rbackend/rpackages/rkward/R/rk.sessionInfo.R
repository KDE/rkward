#' Print information on the RKWard session
#' 
#' Gathers and prints information on the setup of the current RKWard session.
#' In general, you should always include this information when reporting a bug
#' in RKWard.
#' 
#' Typically, when reporting a bug, you should use \code{Help->Report Bug...}
#' from the menu. Internally, this will call \code{rk.sessionInfo()}.
#' 
#' @return Returns the object created by \code{sessionInfo()}, invisibly. Note
#'   that this includes only the information on the R portion of the session.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{sessionInfo}}
#' @keywords utilities misc
#' @export
#' @rdname rk.sessionInf
#' @examples
#' 
#' rk.sessionInfo()

"rk.sessionInfo" <- function () {
	cat (.rk.do.plain.call ("getSessionInfo"), sep="\n")
	cat ("R runtime session info:\n")
	print (sessionInfo())
}
