local({
## Prepare
require(devtools)
## Compute
  install_git(
    url="https://github.com/does/not/exist/404.git"
  )
## Print result
rk.header ("Install from git results")
})
