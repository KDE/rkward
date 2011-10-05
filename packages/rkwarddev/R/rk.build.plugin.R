#' Build an RKWard plugin package
#' 
#' @param plugin A character string, path to the plugin package root directory (hint: it's the directory with
#'		the DESCRIPTION file in it).
#' @param check Logical, whether the package should be checked for errors. Always do this before you
#'		publick a package!
#' @param install Logical, whether the built package should also be installed locally.
#' @param R.libs A character string, path to local R packages, used by \code{install} to figure
#'		out where to install to.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' \dontrun{
#' plugin.dir <- rk.plugin.skeleton("MyPlugin", dialog=full.dialog, wizard=full.wizard)
#' rk.build.plugin(plugin.dir, R.libs="~/R", check=TRUE)
#' }

rk.build.plugin <- function(plugin, check=FALSE, install=FALSE, R.libs=NULL){
	jmp.back <- getwd()
	plugin.path <- normalizePath(plugin)
	build.path <- normalizePath(file.path(plugin.path, ".."))
	plugin.desc.file <- file.path(plugin.path, "DESCRIPTION")
	# read info from DESCRIPTION or die
	if(file_test("-f", plugin.desc.file)){
		plugin.desc <- read.dcf(plugin.desc.file)
	} else {
		stop(simpleError("Invalid plugin: No DESCRIPTION file found!"))
	}
	plugin.name <- as.character(plugin.desc[,"Package"])
	plugin.version <- as.character(plugin.desc[,"Version"])
	pckg.basename <- paste(plugin.name, "_", plugin.version, sep="")
	pckg.name.src <- paste(pckg.basename, ".tar.gz", sep="")
	package.path <- file.path(build.path, pckg.name.src)
	R.bin <- R.home(file.path("bin","R"))

	if(isTRUE(check)){
		# check for examples check file before
		chk.ex.file <- file.path(plugin.path, paste(plugin.name, "-Ex.R", sep=""))
		chk.ex.file.present <- ifelse(file_test("-f", chk.ex.file), TRUE, FALSE)
		tryCatch(chk.out.dir <- tempdir(), error=function(e) stop(e))
		setwd(chk.out.dir)
		set.R.libs <- ifelse(is.null(R.libs), "", paste("R_LIBS_USER=", R.libs, " ; ", sep=""))
		r.cmd.check.call <- paste(set.R.libs, R.bin, " CMD check ", plugin.path, " || exit 1", sep="")
		message(paste("check: calling R CMD check, this might take a while...", sep=""))
		print(system(r.cmd.check.call, intern=TRUE))
		on.exit(message(paste("check: results were saved to ", chk.out.dir, "/", plugin.name, ".Rcheck", sep="")), add=TRUE)
		setwd(jmp.back)
		# need to clean up?
		if(!isTRUE(chk.ex.file.present) & file_test("-f", chk.ex.file)){
			# there's an example file which wasn't here before
			unlink(chk.ex.file)
		} else {}
	} else {}

	setwd(build.path)
	r.cmd.build.call <- paste(R.bin, " CMD build --no-vignettes ", plugin.path, " || exit 1", sep="")
	system(r.cmd.build.call, intern=TRUE)
	message(paste("build: package built as ", package.path, sep=""))

	if(isTRUE(install)){
		install.packages(package.path, lib=R.libs, repos=NULL)
		message("build: package installed.")
	} else {}

	setwd(jmp.back)

	return(invisible(NULL))
}
