#' Create RKWard help file skeleton
#'
#' @param summary An object of class \code{XiMpLe.node} to be pasted as the \code{<summary>} section. See
#'		\code{\link[rkwarddev:rk.rkh.summary]{rk.rkh.summary}} for details.
#' @param usage An object of class \code{XiMpLe.node} to be pasted as the \code{<usage>} section. See
#'		\code{\link[rkwarddev:rk.rkh.usage]{rk.rkh.usage}} for details.
#' @param sections A (list of) objects of class \code{XiMpLe.node} to be pasted as \code{<section>} sections. See
#'		\code{\link[rkwarddev:rk.rkh.section]{rk.rkh.section}} for details.
#' @param settings An object of class \code{XiMpLe.node} to be pasted as the \code{<settings>} section. See
#'		\code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}} for details.
#'		Refer to \code{\link{rk.rkh.scan}} for a function to create this from an existing plugin XML file.
#' @param related An object of class \code{XiMpLe.node} to be pasted as the \code{<related>} section. See
#'		\code{\link[rkwarddev:rk.rkh.related]{rk.rkh.related}} for details.
#' @param technical An object of class \code{XiMpLe.node} to be pasted as the \code{<technical>} section. See
#'		\code{\link[rkwarddev:rk.rkh.technical]{rk.rkh.technical}} for details.
#' @param title An object of class \code{XiMpLe.node} to be pasted as the \code{<title>} section. See
#'		\code{\link[rkwarddev:rk.rkh.title]{rk.rkh.title}} for details.
#' @return An object of class \code{XiMpLe.doc}.
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.summary]{rk.rkh.summary}},
#'		\code{\link[rkwarddev:rk.rkh.usage]{rk.rkh.usage}},
#'		\code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}},
#'		\code{\link{rk.rkh.scan}},
#'		\code{\link[rkwarddev:rk.rkh.related]{rk.rkh.related}},
#'		\code{\link[rkwarddev:rk.rkh.technical]{rk.rkh.technical}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.rkh.doc <- function(summary=NULL, usage=NULL, sections=NULL, settings=NULL, related=NULL, technical=NULL, title=NULL){

	all.children <- list()

	if(!is.null(title)){
		# check if this is *really* a title section
		if(inherits(title, "XiMpLe.node")){
			title.node.name <- title@name
		} else {
			title.node.name <- "yougottabekiddingme"
		}
		if(!identical(title.node.name, "title")){
			stop(simpleError("I don't know what this is, but 'title' is not a title section!"))
		} else {}
		all.children[[length(all.children)+1]] <- title
	} else {}

	if(is.null(summary)){
		all.children[[length(all.children)+1]] <- rk.rkh.summary()
	} else {
		# check if this is *really* a summary section
		if(inherits(summary, "XiMpLe.node")){
			summary.node.name <- summary@name
		} else {
			summary.node.name <- "yougottabekiddingme"
		}
		if(!identical(summary.node.name, "summary")){
			stop(simpleError("I don't know what this is, but 'summary' is not a summary section!"))
		} else {}
		all.children[[length(all.children)+1]] <- summary
	}

	if(is.null(usage)){
		all.children[[length(all.children)+1]] <- rk.rkh.usage()
	} else {
		# check if this is *really* a usage section
		if(inherits(usage, "XiMpLe.node")){
			usage.node.name <- usage@name
		} else {
			usage.node.name <- "yougottabekiddingme"
		}
		if(!identical(usage.node.name, "usage")){
			stop(simpleError("I don't know what this is, but 'usage' is not a usage section!"))
		} else {}
		all.children[[length(all.children)+1]] <- usage
	}

	if(is.null(sections)){
		all.children[[length(all.children)+1]] <- new("XiMpLe.node",
				name="!--",
				children=list(rk.rkh.section("EDIT OR DELETE ME", text="EDIT OR DELETE ME")))
	} else {
		for(this.section in sections){
			# check if this is *really* a section
			if(inherits(this.section, "XiMpLe.node")){
				this.section.node.name <- this.section@name
			} else {
				this.section.node.name <- "yougottabekiddingme"
			}
			if(!identical(this.section.node.name, "section")){
				stop(simpleError("I don't know what this is, but 'sections' does not hold section nodes!"))
			} else {}

			all.children[[length(all.children)+1]] <- this.section
		}
	}

	if(is.null(settings)){
		all.children[[length(all.children)+1]] <- rk.rkh.settings()
	} else {
		# check if this is *really* a settings section
		if(inherits(settings, "XiMpLe.node")){
			settings.node.name <- settings@name
		} else {
			settings.node.name <- "yougottabekiddingme"
		}
		if(!identical(settings.node.name, "settings")){
			stop(simpleError("I don't know what this is, but 'settings' is not a settings section!"))
		} else {}
		all.children[[length(all.children)+1]] <- settings
	}

	if(is.null(related)){
		all.children[[length(all.children)+1]] <- new("XiMpLe.node",
 				name="!--",
 				children=list(rk.rkh.related( rk.rkh.link("..."))))
	} else {
		# check if this is *really* a related section
		if(inherits(related, "XiMpLe.node")){
			related.node.name <- related@name
		} else {
			related.node.name <- "yougottabekiddingme"
		}
		if(!identical(related.node.name, "related")){
			stop(simpleError("I don't know what this is, but 'related' is not a related section!"))
		} else {}
		all.children[[length(all.children)+1]] <- related
	}

	if(is.null(technical)){
		all.children[[length(all.children)+1]] <- rk.rkh.technical()
	} else {
		# check if this is *really* a technical section
		if(inherits(technical, "XiMpLe.node")){
			technical.node.name <- technical@name
		} else {
			technical.node.name <- "yougottabekiddingme"
		}
		if(!identical(technical.node.name, "technical")){
			stop(simpleError("I don't know what this is, but 'technical' is not a technical section!"))
		} else {}
		all.children[[length(all.children)+1]] <- technical
	}

	rkh.document <- new("XiMpLe.node",
			name="document",
			children=all.children,
			value="")

	rkh.main <- new("XiMpLe.doc",
			dtd=list(doctype="rkhelp"),
			children=list(rkh.document))

	return(rkh.main)
}
