\name{rk.record.commands}
\alias{rk.record.commands}
\title{Start recording commands that are submitted from RKWard to R}
\usage{
  rk.record.commands(filename, include.all = FALSE)
}
\arguments{
  \item{filename}{filename to write to (file will be
  truncated!).}

  \item{include.all}{By default, some types of command are
  filtered (internal synchronisation commands, and run
  again links). Should these be included?}
}
\description{
  To stop recording, supply NULL or "" as filename.
  Currently used for the purpose of automated testing,
  only. Perhaps in the future this or a similar mechanism
  could also be added as a user feature.
}

