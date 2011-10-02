\name{rk.rkh.doc}
\alias{rk.rkh.doc}
\title{Create RKWard help file skeleton}
\usage{
  rk.rkh.doc(summary = NULL, usage = NULL, settings = NULL,
  related = NULL, technical = NULL)
}
\arguments{
  \item{summary}{A list of objects of class
  \code{XiMpLe.node} or a character string with the content
  of the Summary section.}

  \item{usage}{A list of objects of class
  \code{XiMpLe.node} or a character string with the content
  of the Usage section.}

  \item{settings}{A list of objects of class
  \code{XiMpLe.node} with the content of the Settings
  section. Refer to \code{\link{rk.rkh.scan}} for a
  function to create this from an existing plugin XML
  file.}

  \item{related}{A list of objects of class
  \code{XiMpLe.node} or a character string with the content
  of the Related section.}

  \item{technical}{A list of objects of class
  \code{XiMpLe.node} or a character string with the content
  of the technical section.}
}
\value{
  An object of class \code{XiMpLe.doc}.
}
\description{
  Create RKWard help file skeleton
}

