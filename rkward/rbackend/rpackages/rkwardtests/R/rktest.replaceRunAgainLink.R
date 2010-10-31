#' Replace "Run again" link in RKWard with code
#'
#' You can use this to temporarily replace .rk.rerun.plugin.link (see example below).
#' This way, after running a plugin, you are shown the call needed to run this
#' plugin with those settings, instead of the link.
#'
#' This code can be used in a plugin test suite.
#' 
#' @title Replace "Run again" link in RKWard
#' @usage rktest.replaceRunAgainLink(restore=FALSE)
#' @aliases .rk.rerun.plugin.link.replacement
#' @param restore Logical: If TRUE, restore the original behaviour.
#' @return Replaces the "Run again" link in RKWard with the code that would have been called, or vice versa.
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@@uni-duesseldorf.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @rdname rktest.replaceRunAgainLink
#' @examples
#' rktest.replaceRunAgainLink()

rktest.replaceRunAgainLink <- function(restore=FALSE){
	if(!restore){
		rktest.replace (".rk.rerun.plugin.link", .rk.rerun.plugin.link.replacement, backup.name=".rk.rerun.plugin.link.manual.replace")
	} else {
		rktest.restore (".rk.rerun.plugin.link", backup.name=".rk.rerun.plugin.link.manual.replace")
	}
}
