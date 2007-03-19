<?
function preprocess () {
}

function calculate () {
	if (getRK_val ("other_env")) {
		$other_env = true;
		$envir = getRK_val ("envir");
	} else {
		$envir = "globalenv()";
	}

	if ($other_env) { ?>
<? echo ($envir); ?> <<- new.env (parent=globalenv())
<?	} ?>
load (file="<? getRK("file"); ?>", envir=<? echo ($envir); ?>)
<?
}

function printout () {
}

?>
