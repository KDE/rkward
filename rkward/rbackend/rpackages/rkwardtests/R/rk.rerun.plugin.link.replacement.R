#' Replace "Run again" link in RKWard with code
#'
#' You can use this to temporarily replace .rk.rerun.plugin.link (see example below).
#' This way, after running a plugin, you are shown the call needed to run this
#' plugin with those settings, instead of the link.
#'
#' This code can be used in a plugin test suite.
#' 
#' @title Replace "Run again" link in RKWard
#' @usage .rk.rerun.plugin.link <- .rk.rerun.plugin.link.replacement
#' @aliases .rk.rerun.plugin.link.replacement
#' @param plugin (used internally)
#' @param settings (used internally)
#' @param label (used internally)
#' @return Replaces the "Run again" link in RKWard with the code that would have been called.
#' @docType function
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @rdname rk.rerun.plugin.link.replacement
#' @examples
#' \dontrun{
#' # NOTE: Do NOT end the function with brackets, as its code has to be
#' # written into .rk.rerun.plugin.link
#' .rk.rerun.plugin.link <- .rk.rerun.plugin.link.replacement
#' }

.rk.rerun.plugin.link.replacement <- function (plugin, settings, label) {
	.rk.cat.output ("<h3>Rerun code:</h3>")
	.rk.cat.output ("<pre>")
	.rk.cat.output ("rk.call.plugin (\"")
	.rk.cat.output (plugin)
	.rk.cat.output ("\", ")
	.rk.cat.output (gsub ("^\"", "", gsub ("=", "=\"", gsub ("\n", "\", ", settings))))
	.rk.cat.output ("\", submit.mode=\"submit\")</pre>")
}


