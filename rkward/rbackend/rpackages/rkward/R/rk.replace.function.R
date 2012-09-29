#' Replace a function inside its package environment / namespace
#' 
#' \code{rk.replace.function} can be used to replace a function inside a
#' different package / namespace. It is mainly intended for internal usage
#' inside rkward, e.g. to replace \code{menu} and \code{select.list} with
#' appropriate GUI implementations.
#' 
#' The original function is assigned to the environment
#' \code{rkward::.rk.backups} with the same name as the original, and can be
#' referred to from the replacement. WARNING: This mechanism does not support
#' several subsequent replacments of the same function, nor does it support
#' replacement of generics.
#' 
#' \bold{WARNING}: This function can be used to alter - and disrupt - internal
#' functions in arbitrary ways. You better know what you are doing.
#' 
#' \bold{WARNING}: Does not work well on generics!
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
	# This is a stripped down copy of utils::assignInNamespace, without the restrictions
	# added which would prevent us from properly replacing e.g. utils::menu,
	# but also without some of the fine points for replacing while loading a namespace,
	# and for handling S3 methods.
	doAssignInNamespace <- function(x, value, ns, pos = -1, envir = as.environment(pos))
	{
		if (missing(ns)) {
			nm <- attr(envir, "name", exact = TRUE)
			if(is.null(nm) || substring(nm, 1L, 8L) != "package:")
			    stop("environment specified is not a package")
			ns <- asNamespace(substring(nm, 9L))
		} else ns <- asNamespace(ns)
		if (bindingIsLocked(x, ns)) {
			unlockBinding(x, ns)
			assign(x, value, envir = ns, inherits = FALSE)
			w <- options("warn")
			on.exit(options(w))
			options(warn = -1)
			lockBinding(x, ns)
		} else {
			assign(x, value, envir = ns, inherits = FALSE)
		}
	}

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
			doAssignInNamespace (functionname, replacement, ns=environment)
		} else {
			doAssignInNamespace (functionname, replacement, envir=environment)
		}
	)
	try (
		assign (functionname, replacement, envir=environment)
	)

	invisible (NULL)
}
