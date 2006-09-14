<?
	function preprocess () {
	}
	
	function calculate () {
	}
	
	function printout () {
		$fun = getRK_val ("function");
		if ($fun == "dbinom") $label = "density";
		else $label = "distribution";
	
		$min = getRK_val ("min");
		$nq = getRK_val ("nq");
		$space = getRK_val ("space");
		$size = getRK_val ("size");
		$prob = getRK_val ("prob");
	
		if (getRK_val ("log") == "1") {
			if ($fun == "dbinom") {
				$log = ", log=TRUE";
			} else {
				$log = ", log.p=TRUE";
			}
			$log_label="logarithmic";
		} else {
			$log = "";
			$log_label="normal";
		}

?>rk.header ("Plot binomial <? echo ($label); ?>", list ("Lowest quantile", "<? echo ($min); ?>", "Largest quantile", "<? echo ($min + ($nq * $space)); ?>", "Quantile spacing", "<? echo ($space); ?>", "Number of trials", "<? echo ($size); ?>", "Probability of success on each trial", "<? echo ($prob); ?>", "Function", "<? getRK ("function"); ?>", "Scaling", "<? echo ($log_label); ?>"));

rk.graph.on ()
plot (<? getRK ("function"); ?> (seq (<? echo ($min); ?>, <? echo ($min + ($nq * $space)); ?>, by=<? echo ($space); ?>) , size=<? echo ($size); ?>, prob=<? echo ($prob); echo ($log); ?>))
rk.graph.off ()
<?
	}
	
	function cleanup () {
	}
?>
