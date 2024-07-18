# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Install or update RKWard AppImage file
#'
#' Tries to download the latest AppImage of RKWard to a specified directory and
#' makes it executable so it can be used for launching RKWard.
#' 
#' This function uses functions of the \code{XiMpLe} package, allthough for technical
#' reasons that package could not be declared a proper dependency. This means, if
#' you use this function for the first time with the current R version and hadn't
#' installed \code{XiMpLe} yet, you will be prompted to install it first. This
#' behaviour is similar to using an RKWard plugin dialog with a \code{require()}
#' call to a currently not installed package.
#'
#' @param dir File path to the target directory.
#' @param filename Character string, filename for the downloaded AppImage.
#' @param download Logical, whether to actually download the file or only return the full URL to it in a character string.
#' @param overwrite Logical, should an existing file be overwritten?
#' @param url Base URL to download the AppImage from.
#' @param pattern Regular expression to find the AppImage file name in \code{url}.
#' @param method See \code{\link[utils:download.file]{download.file}}.
#' @param cacheOK See \code{\link[utils:download.file]{download.file}}.
#' @param timeout Number of seconds to try finishing the download.
#' @param ... Additionsl options for \code{\link[utils:download.file]{download.file}}.
#' @return If \code{download = FALSE} returns the direct URL to the AppImage file.
#'    Otherwise returns the invisible file path to the installed AppImage if all went well,
#'    or invisible \code{NULL} if something went wrong.
#' @author Meik Michalke \email{rkward-devel@@kde.org}
#' @keywords utilities misc
# # don't create additional dependencies for the rkward package
# @importFrom XiMpLe parseXMLTree XMLScanDeep
# @importFrom utils download.file
#' @rdname rk.download_appimage
#' @export
#' @examples
#' \dontrun{
#' # install current AppImage from master branch to ~/bin
#' rk.download_appimage(dir="~/bin")
#'
#' # update the AppImage
#' rk.download_appimage(dir="~/bin", overwrite=TRUE)
#' }

rk.download_appimage <- function(
    dir = dirname(Sys.getenv("APPIMAGE"))
  , filename = "rkward-master-linux-gcc-x86_64.AppImage"
  , download = TRUE
  , overwrite = FALSE
  , url = "https://cdn.kde.org/ci-builds/education/rkward/master/linux"
  , pattern = "rkward-master.*linux-gcc-x86_64\\.AppImage"
  , method = "auto"
  , cacheOK = FALSE
  , timeout =  max(400, getOption("timeout"))
  , ...
){

  # we are well aware that require() is not the method of choice
  # this call will invoke rkward::require(), which should only be present
  # in a running session of RKWard, and therefore ask for the installation
  # of XiMpLe if the package is currently missing in the R version that
  # is running at the moment
  rkward::require(XiMpLe)
  XiMpLe_version <- utils::packageVersion("XiMpLe")
  if(isFALSE(XiMpLe_version >= "0.11.3")){
    stop(simpleError(paste0("rk.download_appimage() requires XiMpLe >= 0.11.3, but only found ", XiMpLe_version, ". Please update XiMpLe.")))
  } else {}
  rk_ai_html <- XiMpLe::parseXMLTree(url, drop="empty_attributes")
  rk_ai_hrefs <- XiMpLe::XMLScanDeep(rk_ai_html, find="href")
  rk_ai_file <- rk_ai_hrefs[grepl(pattern=pattern, x=rk_ai_hrefs)][[1]]

  rk_ai_full_url <- file.path(url, rk_ai_file)

  if(isTRUE(download)){
    if(!dir.exists(dir)){
      stop(simpleError(paste0("Target directory does not exist:\n  ", dir)))
    } else {}
    have_backup <- FALSE
    target_file <- file.path(dir, filename)
    if(file.exists(target_file)){
      if(isFALSE(overwrite)){
        stop(simpleError(paste0("Target file already exists, use the \"overwrite\" argument to replace it:\n  ", target_file)))
      } else {}
      # create backup of current AppImage as a fallback
      rk_ai_backup <- paste0(target_file, ".backup_", format(Sys.time(), "%Y-%m-%d_%H%M%S"))
      message(paste0("Creating backup of current AppImage:\n  ", rk_ai_backup))
      file.rename(
          from=target_file
        , to=rk_ai_backup
      )
      have_backup <- TRUE
    } else {}
    timeout_orig <- getOption("timeout")
    options(timeout = timeout)
    dl_status <- tryCatch(
        # value should be 0 if download was successful
        utils::download.file(
            url = rk_ai_full_url
          , destfile = target_file
          , method = method
          , cacheOK = cacheOK
          , quiet = FALSE
          , ...
        )
      , error = function(cond){
          if(isTRUE(have_backup)){
            file.rename(
                from=rk_ai_backup
              , to=target_file
            )
            stop("Something went wrong with the download! Restored previous AppImage from backup.", call.=FALSE)
          } else {
            file.remove(target_file)
            stop("Something went wrong with the download!", call.=FALSE)
          }
        }
      , finally = options(timeout = timeout_orig)
    )
    if(isTRUE(dl_status == 0)){
      message("Download successful, setting file permissions.")
      Sys.chmod(
          paths=target_file
        , mode="0755"
        , use_umask=TRUE
      )
      if(isTRUE(have_backup)){
        message("Removing backup of previous AppImage.")
        file.remove(rk_ai_backup)
      } else {}
      return(invisible(target_file))
    } else {
      if(isTRUE(have_backup)){
        warning("Something went wrong with the download! Restoring previous AppImage from backup.", call.=FALSE)
        file.rename(
            from=rk_ai_backup
          , to=target_file
        )
      } else {
        warning("Something went wrong with the download!", call.=FALSE)
        file.remove(target_file)
      }
    }
    return(invisible(NULL))
  } else {
    return(rk_ai_full_url)
  }
}
