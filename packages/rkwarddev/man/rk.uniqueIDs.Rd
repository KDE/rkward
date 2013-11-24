\name{rk.uniqueIDs}
\alias{rk.uniqueIDs}
\title{Check plugin dialogs for duplicate IDs}
\usage{
  rk.uniqueIDs(obj, bool = FALSE, warning = TRUE)
}
\arguments{
  \item{obj}{An XML object of class XiMpLe.node or
  XiMpLe.doc.}

  \item{bool}{Logical, if \code{TRUE} this function will
  return a logical value.}

  \item{warning}{Logical, if \code{TRUE} will throw a
  warning if duplicates were found, listing the findings.}
}
\value{
  A vector with duplicate IDs, if any were found.  If
  \code{bool=TRUE} returns a logical value.
}
\description{
  A plugin must not have duplicated IDs to work properly.
  This function cannot automatically correct duplicates,
  but it will warn you about it, so you can correct your
  plugin script manually
}

