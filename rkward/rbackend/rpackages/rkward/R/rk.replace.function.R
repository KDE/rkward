#' Replace a function inside its package environment / namespace
#' 
#' \code{rk.replace.function} can be used to replace a function inside a
#' different package / namespace. It is mainly intended for internal usage
#' inside rkward, e.g. to replace \code{menu} and \code{select.list} with
#' appropriate GUI implementations.
#' 
#' The original function is assigned to the environment
#' \code{rkward::.rk.backups} with the same name as the original, and can be
#' referred to from the replacement. WARNING: This mechansim does not support
#' several subsequent replacments of the same function.
#' 
#' WARNING: This function can be used to alter - and disrupt - internal
#' functions in arbitrary ways. You better know what you are doing.
#' 
#' WARNING: Does not work well on generics!
#' 
#' @param functionname name of the function to be replaced (character).
#' @param environment package environment or namespace, where replacment should
#'   be done.
#' @param replacement the replacement. This should be a function.
#' @param copy.formals logical; whether to copy the \code{\link{formals}} from
#'   the original function.
#' @return Returns \code{NULL}, invisibly, unconditionally.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{assignInNamespace}}, \code{\link{debug}}
#' @keywords utilities IO
#' @export
#' @rdname rk.replace.function
#' @examples
#' 
#' ## Not run
#' rk.replace.function ("history", as.environment ("package:utils"),
#'   function () {
#'     cat ("This is what you typed:\n")
#'     eval (body (.rk.backups$history))
#'   })
#' ## End not run
#' 

# Tries to replace a function inside its environemnt/namespace.
# Function formals are copied from the original.
# A backup of the original is stored as rkward::.rk.backups$FUNCTIONNAME
"rk.replace.function" <- function (functionname, environment, replacement, copy.formals=TRUE) {
	original <- get (functionname, envir=environment, inherits=FALSE)

	# create a backup
	assign (functionname, original, envir=.rk.backups)

	if (copy.formals) formals (replacement) <- formals (original)
	environment (replacement) <- environment (original)
	try (
		if (bindingIsLocked (functionname, environment)) {
			unlockBinding (functionname, environment)
			on.exit (lockBinding (functionname, environment))
		}
	)
	try (
		if (isNamespace (environment)) {
			assignInNamespace (functionname, replacement, ns=environment)
		} else {
			assignInNamespace (functionname, replacement, envir=environment)
		}
	)
	try (
		assign (functionname, replacement, envir=environment)
	)

	invisible (NULL)
}
