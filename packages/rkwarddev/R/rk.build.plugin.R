#' Build an RKWard plugin package
#' 
#' @param plugin A character string, path to the plugin package root directory (hint: it's the directory with
#'		the DESCRIPTION file in it).
#' @param check Logical, whether the package should be checked for errors. Always do this before you
#'		publish a package!
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
	pckg.basename <- paste0(plugin.name, "_", plugin.version)
	pckg.name.src <- paste0(pckg.basename, ".tar.gz")
	package.path <- file.path(build.path, pckg.name.src)
	R.bin <- R.home(file.path("bin","R"))

	if(isTRUE(check)){
		# check for examples check file before
		chk.ex.file <- file.path(plugin.path, paste0(plugin.name, "-Ex.R"))
		chk.ex.file.present <- ifelse(file_test("-f", chk.ex.file), TRUE, FALSE)
		tryCatch(chk.out.dir <- tempdir(), error=function(e) stop(e))
		setwd(chk.out.dir)
		set.R.libs <- ifelse(is.null(R.libs), "", paste0("R_LIBS_USER=", R.libs, " ; "))
		r.cmd.check.call <- paste0(set.R.libs, R.bin, " CMD check ", plugin.path, " || exit 1")
		message(paste0("check: calling R CMD check, this might take a while..."))
		print(system(r.cmd.check.call, intern=TRUE))
		on.exit(message(paste0("check: results were saved to ", chk.out.dir, "/", plugin.name, ".Rcheck")), add=TRUE)
		setwd(jmp.back)
		# need to clean up?
		if(!isTRUE(chk.ex.file.present) & file_test("-f", chk.ex.file)){
			# there's an example file which wasn't here before
			unlink(chk.ex.file)
		} else {}
	} else {}

	setwd(build.path)
	r.cmd.build.call <- paste0(R.bin, " CMD build --no-vignettes ", plugin.path, " || exit 1")
	system(r.cmd.build.call, intern=TRUE)
	message(paste0("build: package built as ", package.path))

	if(isTRUE(install)){
		install.packages(package.path, lib=R.libs, repos=NULL)
		message("build: package installed.")
	} else {}

	setwd(jmp.back)

	return(invisible(NULL))
}
