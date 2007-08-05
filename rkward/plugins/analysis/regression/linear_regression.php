<?
function preprocess () {
        }

function calculate () {
?>
results <- summary (lm (<? getRK ("y"); ?> ~ <? getRK ("x"); ?>))
<?
}

function printout () {
?>
rk.header ("Linear Regression")
rk.print (results)
<?
}
?>