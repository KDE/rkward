<?php
function preprocess () {
}
function calculate () {
  $dir = getRK_val ("dir");
?>
setwd("<? echo ($dir); ?>")
<?
}
function printout () {
}
function cleanup () {
}
?>