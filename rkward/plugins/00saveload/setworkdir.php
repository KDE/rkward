<?php
function preprocess () {
}
function calculate () {
  $dir = getRK_val ("dir");
  if (is_dir($dir)) {
?>
setwd("<? echo ($dir); ?>")
<?
  } else {
?>
cat("Not a directory.")
<?
  }
}
function printout () {
}
function cleanup () {
}
?>