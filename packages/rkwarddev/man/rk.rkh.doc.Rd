\name{rk.rkh.doc}
\alias{rk.rkh.doc}
\title{Create RKWard help file skeleton}
\usage{
  rk.rkh.doc(summary = NULL, usage = NULL, sections = NULL,
  settings = NULL, related = NULL, technical = NULL, title
  = NULL, gen.info = TRUE)
}
\arguments{
  \item{summary}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<summary>} section. See
  \code{\link[rkwarddev:rk.rkh.summary]{rk.rkh.summary}}
  for details.}

  \item{usage}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<usage>} section. See
  \code{\link[rkwarddev:rk.rkh.usage]{rk.rkh.usage}} for
  details.}

  \item{sections}{A (list of) objects of class
  \code{XiMpLe.node} to be pasted as \code{<section>}
  sections. See
  \code{\link[rkwarddev:rk.rkh.section]{rk.rkh.section}}
  for details.}

  \item{settings}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<settings>} section. See
  \code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}}
  for details. Refer to \code{\link{rk.rkh.scan}} for a
  function to create this from an existing plugin XML
  file.}

  \item{related}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<related>} section. See
  \code{\link[rkwarddev:rk.rkh.related]{rk.rkh.related}}
  for details.}

  \item{technical}{An object of class \code{XiMpLe.node} to
  be pasted as the \code{<technical>} section. See
  \code{\link[rkwarddev:rk.rkh.technical]{rk.rkh.technical}}
  for details.}

  \item{title}{An object of class \code{XiMpLe.node} to be
  pasted as the \code{<title>} section. See
  \code{\link[rkwarddev:rk.rkh.title]{rk.rkh.title}} for
  details.}

  \item{gen.info}{Logical, if \code{TRUE} a comment note
  will be written into the document, that it was generated
  by \code{rkwarddev} and changes should be done to the
  script.}
}
\value{
  An object of class \code{XiMpLe.doc}.
}
\description{
  Create RKWard help file skeleton
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.summary]{rk.rkh.summary}},
  \code{\link[rkwarddev:rk.rkh.usage]{rk.rkh.usage}},
  \code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}},
  \code{\link{rk.rkh.scan}},
  \code{\link[rkwarddev:rk.rkh.related]{rk.rkh.related}},
  \code{\link[rkwarddev:rk.rkh.technical]{rk.rkh.technical}}
  and the \href{help:rkwardplugins}{Introduction to Writing
  Plugins for RKWard}
}

