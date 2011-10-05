#' Create RKWard help file skeleton
#'
#' Create RKWard help file skeleton
#'
#' @param summary A list of objects of class \code{XiMpLe.node} or a character string with the content of the Summary section.
#' @param usage A list of objects of class \code{XiMpLe.node} or a character string with the content of the Usage section.
#' @param settings A list of objects of class \code{XiMpLe.node} with the content of the Settings section.
#'		Refer to \code{\link{rk.rkh.scan}} for a function to create this from an existing plugin XML file.
#' @param related A list of objects of class \code{XiMpLe.node} or a character string with the content of the Related section.
#' @param technical A list of objects of class \code{XiMpLe.node} or a character string with the content of the technical section.
#' @return An object of class \code{XiMpLe.doc}.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.rkh.doc <- function(summary=NULL, usage=NULL, settings=NULL, related=NULL, technical=NULL){

	rkh.summary <- new("XiMpLe.node",
			name="summary")
	if(is.null(summary)){
		rkh.summary@value <- ""
	} else if(is.list(summary)){
		rkh.summary@children <- summary
	} else if(inherits(summary, "XiMpLe.node")){
		rkh.summary@children <- list(summary)
	} else {
		rkh.summary@value <- summary
	}

	rkh.usage <- new("XiMpLe.node",
			name="usage")
	if(is.null(usage)){
		rkh.usage@value <- ""
	} else if(is.list(usage)){
		rkh.usage@children <- usage
	} else if(inherits(usage, "XiMpLe.node")){
		rkh.usage@children <- list(usage)
	} else {
		rkh.usage@value <- usage
	}

	rkh.settings <- new("XiMpLe.node",
			name="settings")
	if(is.null(settings)){
		rkh.settings@value <- ""
	} else if(is.list(settings)){
		rkh.settings@children <- settings
	} else if(inherits(settings, "XiMpLe.node")){
		rkh.settings@children <- list(settings)
	} else {
		rkh.settings@value <- settings
	}

	rkh.related <- new("XiMpLe.node",
			name="related")
	if(is.null(related)){
		rkh.related@children <- list(new("XiMpLe.node",
				name="!--",
				value="<ul><li><link href=\"rkward://rhelp/...\"/></li></ul>"))
	} else if(is.list(related)){
		rkh.related@children <- related
	} else if(inherits(related, "XiMpLe.node")){
		rkh.related@children <- list(related)
	} else {
		rkh.related@value <- related
	}

	rkh.technical <- new("XiMpLe.node",
			name="technical")
	if(is.null(technical)){
		rkh.technical@value <- ""
	} else if(is.list(technical)){
		rkh.technical@children <- technical
	} else if(inherits(technical, "XiMpLe.node")){
		rkh.technical@children <- list(technical)
	} else {
		rkh.technical@value <- technical
	}

	rkh.document <- new("XiMpLe.node",
			name="document",
			children=list(rkh.summary, rkh.usage, rkh.settings, rkh.related, 	rkh.technical),
			value="")

	rkh.main <- new("XiMpLe.doc",
			dtd=list(doctype="rkhelp"),
			children=list(rkh.document))

	return(rkh.main)
}

# <!DOCTYPE rkhelp>
# 	<document>
# 		<summary>
# 		</summary>
# 
# 		<usage>
# 		</usage>
# 
# 		<settings>
# 			<caption id="tab_klausur"/>
# 			<setting id="antworten">
# 			</setting>
# 			<setting id="richtig" title="Correct answers">
# 			</setting>
# 		</settings>
# 
# 		<related>
# 			Please refer to the <code>klausuR</code> manuals for further information and detailed command line options:
# 			<ul>
# 						<li><link href="rkward://rhelp/klausuR-package"/></li>
# 						<li><link href="rkward://rhelp/klausur"/></li>
# 						<li><link href="rkward://rhelp/klausur.report"/></li>
# 			</ul>
# 		</related>
# 
# 		<technical>
# 		</technical>
# 	</document>
