# Copyright 2010-2014 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package rkwarddev.
#
# rkwarddev is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rkwarddev is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rkwarddev.  If not, see <http://www.gnu.org/licenses/>.


#' Check plugin dialogs for duplicate IDs
#'
#' A plugin must not have duplicated IDs to work properly. This function
#' cannot automatically correct duplicates, but it will warn you about
#' it, so you can correct your plugin script manually
#'
#' @param obj An XML object of class XiMpLe.node or XiMpLe.doc.
#' @param bool Logical, if \code{TRUE} this function will return a logical value.
#' @param warning Logical, if \code{TRUE} will throw a warning if duplicates were found,
#'    listing the findings.
#' @param ignore Character vector, node names that should generally be ignored because
#'    they duplicate IDs by design.
#' @return A vector with duplicate IDs, if any were found.
#'    If \code{bool=TRUE} returns a logical value.

rk.uniqueIDs <- function(obj, bool=FALSE, warning=TRUE, ignore=c("copy")){
  # plugins probably use identical IDs in <dialog> and <wizard> sections
  # so we first check without the <wizard> section separately, then
  # without the <dialog>
  haveDialog <- XMLScan(obj, name="dialog")
  haveWizard <- XMLScan(obj, name="wizard")
  if(all(!is.null(haveDialog), !is.null(haveWizard))){
    noWizard <- noDialog <- obj
    XMLScan(noWizard, name="wizard") <- NULL
    XMLScan(noDialog, name="dialog") <- NULL
    allResults <- lapply(
      list(noWizard, noDialog),
      function(thisPart){
        rk.uniqueIDs(obj=thisPart, bool=bool, warning=warning, ignore=ignore)
      }
    )
    if(isTRUE(bool)){
      return(all(allResults))
    } else {
      return(unlist(allResults))
    }
  } else {
    allIDs <- XMLScanDeep(obj, find="id", search="attributes")
    # ignore nodes
    allIDs <- allIDs[!names(allIDs) %in% ignore]
    duplicateIDs <- duplicated(allIDs)
    if(any(duplicateIDs)){
      # ok, let's get 'em
      invalidIDs <- unique(allIDs[duplicateIDs])
      result <- allIDs[allIDs %in% invalidIDs]
      if(isTRUE(warning)){
        warning(paste0("Duplicate IDs were found:\n  ", paste0(names(result), ": ", result, collapse="\n  ")), call.=FALSE)
      } else {}
      if(isTRUE(bool)){
        return(FALSE)
      } else {
        return(result)
      }
    } else {
      if(isTRUE(bool)){
        return(TRUE)
      } else {
        return(invisible(NULL))
      }
    }
  }
}

