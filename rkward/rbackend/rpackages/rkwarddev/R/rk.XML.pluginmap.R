#' Write a pluginmap file for RKWard
#'
#' @param name Character string, name of the plugin.
#' @param about A list with descriptive information on the plugin,its authors and dependencies.
#'		See \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for details! Skipped if \code{NULL}.
#' @param components A character vector with at least one plugin component file name,
#'		ending with ".xml".
#' @param plugin.dir Character string, relative path to the component XML and JS files.
#' @param hierarchy A character vector with instructions where to place the plugin in the menu hierarchy,
#'		one string for each included component. Valid values are \code{"analysis"},  \code{"plots"} and  
#'		\code{"data"}. To place your dialogs somewhere else, edit the pluginmap manually.
#' @param clean.name Logical, if \code{TRUE}, all non-alphanumeric characters except the underscore (\code{"_"}) will be removed from \code{name}.
#' @export

rk.XML.pluginmap <- function(name, about=NULL, components, plugin.dir="plugins", hierarchy="analysis", clean.name=TRUE){
	if(isTRUE(clean.name)){
		# to besure, remove all non-character symbols from name
		name.orig <- name
		name <- clean.name(name)
	} else {}

	# .pluginmap has three children in <document>:
	# - about
	# - components
	# - hierarchy
	if(!is.null(about)){
	about.XML <- rk.XML.about(
		name=name.orig,
		author=about[["author"]],
		about=about[["about"]],
		dependencies=about[["dependencies"]],
		package=about[["package"]],
		pluginmap=about[["pluginmap"]])
	} else {
		about.XML <- new("XiMpLe.node",
		name="!--",
		value="<about></about>")
	}

	components.XML <- new("XiMpLe.node",
		name="components",
		children=sapply(components, function(this.comp){
				new("XiMpLe.node",
					name="component",
					attributes=list(
						type="standard",
						# if this ID get's a change, also change it in rk.plugin.skeleton(show=TRUE)!
						id=paste(name, ".", gsub(".xml", "", this.comp), sep=""),
						file=paste(plugin.dir, "/", this.comp, sep=""),
						label=gsub(".xml", "", this.comp))
				)})
	)

	hier.comp.XML <- unlist(sapply(1:length(hierarchy), function(this.dial){
		this.comp <- components[this.dial]
		this.hier <- hierarchy[this.dial]
		entry.XML <- new("XiMpLe.node",
			name="menu",
			attributes=list(
				id=paste("menu_", name, ".", gsub(".xml", "", this.comp), sep=""),
				label=name.orig),
			children=list(new("XiMpLe.node",
				name="entry",
				attributes=list(
					component=paste(name, ".", gsub(".xml", "", this.comp), sep=""))
				)))

		if(identical(this.hier, "plots")){
			hier.XML <- new("XiMpLe.node",
				name="menu",
				attributes=list(
					id="plots",
					label="Plots"),
				children=child.list(entry.XML))
		} else if(identical(this.hier, "data")){
			hier.XML <- new("XiMpLe.node",
				name="menu",
				attributes=list(
					id="data",
					label="Data"),
				children=child.list(entry.XML))
		} else {
			hier.XML <- new("XiMpLe.node",
				name="menu",
				attributes=list(
					id="analysis",
					label="Analysis"),
				children=child.list(entry.XML))
		}
	}))

	hierarchy.XML <- new("XiMpLe.node",
		name="hierarchy",
		children=hier.comp.XML)

	top.doc <- new("XiMpLe.node",
		name="document",
		attributes=list(base_prefix="", namespace="rkward", id=paste(name, "_rkward", sep="")),
		children=list(about.XML, components.XML, hierarchy.XML)
	)

	pluginmap <- new("XiMpLe.doc",
			dtd=list(doctype="rkpluginmap"),
			children=child.list(top.doc)
	)

	return(pluginmap)
}
