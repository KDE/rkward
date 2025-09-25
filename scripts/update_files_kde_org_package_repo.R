## Instructions / How to update the files.kde.org/rkward/R repo
# 1. Be sure to edit the main.root.packages var according to your setup
# 2. You will need a *manual* clone of all packages that you want to update in file.path(main.root.packages, "sources")
#    You will need to manually checkout the appropriate branch/tag (and of course you should do at least some very
#    basic checking)
# 3. Enable the packages to update in the for loop, below
#    - if only updating a subset of packages, be sure to work with a copy of the existing repo, in order to create proper index files!
#    - TODO: document this part, properly
# 4. Source/run the whole script. You may/will have to install various additional packages at several points
# 5. Upload to sftp://files.kde.org, internal path rkward/web/R

# This script is based on roxyPackage by Meik Michalke, which can do a lot more package management tasks that just this.

## setup environment
# .libPaths("~/R")
require(roxyPackage)

local({
  main.root.packages <- file.path("/home","thomas","develop","rpackages")
  sandbox(TRUE, pck.source.dir=FALSE, R.libs=FALSE)

    actions <- c(
        "html",           # update HTML index files
        "package"#,         # build & install plugin package
    )

  for (this.plugin in c(
     # https://github.com/rkward-community
     "rk.ANOVA",
     "rk.ClusterAnalysis",
     "rk.CohenKappa",
     "rk.downloadAppImage",
     "rk.FactorAnalysis",
     "rk.gitInstall",
     "rk.MultidimensionalScaling",
     "rk.Teaching",
     "rkwarddev",
#     "rk.MPT", # documented as broken

     # https://github.com/AlfCano
     "rk.transpose.df",
     "rk.cSplit",
     "rk.gsub.sub",
     "rk.forcats",
     "rk.pivot.reshape",
     "rk.gtsummary",
     "rk.survey.design",
     NULL
  )){
    pck.name <- this.plugin
    main.root <- file.path(main.root.packages,"sources")
    repo.root.v <- file.path(main.root.packages,"repo_rkward")
    repo.archive.v <- file.path(main.root,"repo_archive")
    r.dir <- file.path(main.root, pck.name)
    roxy.dir <- r.dir
    R.libs.v <- .libPaths()[1]

    roxy.package(actions=actions,
      pck.description=NULL,
      pck.source.dir=r.dir,
      pck.version=NULL,
      R.libs.append=rk.home("lib"),
      R.libs=.libPaths(),
      repo.root=repo.root.v,
      cleanup=TRUE,
      URL=c(
        default="https://files.kde.org/rkward/R"
      ),
      html.options=list(
        index="Available RKWard Plugin Packages",
        title="RKWard Plugin Package"
      ),
      ChangeLog=list(),
      Rbuildignore=c(".git")
    )
  }
})
