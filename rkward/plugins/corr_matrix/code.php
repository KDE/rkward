<?
	function preprocess () {
	}
	
	function calculate () {
		$method = getRK_val ("method");
		$use = getRK_val ("use");
		$vars = strtr (trim (getRK_val ("x")), "\n", ",");
		
?>
rk.temp <- cor (data.frame (<? echo ($vars); ?>), NULL, "<? echo $use; ?>", "<? echo $method; ?>")
<?
	}
	
	function printout () {
		$method = getRK_val ("method");
		$use = getRK_val ("use");
		$vars = array ();
		$vars = explode ("\n", trim (getRK_val ("x")));
		$labels = array ();
		$labels = explode ("\n", trim (getRK_val ("x.label")));
		
		// fetch values from R
		callR_val ("rk.temp <- data.frame (rk.temp)");
		$matrix = array ();
		for ($i = count ($vars); $i > 0; --$i) {
			$matrix[$i] = array ();
			$matrix[$i] = getRVector ("rk.temp[[" . ($i) . "]]");
		}
		
		// produce the output
?><h1>Correlation Matrix</h1>
<h2>TODO: describe in verbatim: <? echo ($method); ?>, <? echo ($use); ?></h2>
<table border="1">
	<tr>
	<td>Variable</td>
	<? for ($i = 0; $i < count ($vars); ++$i) { ?>
		<td><? echo $vars[$i]; ?></td>
	<? } ?>
	</tr>
<? for ($i = 0; $i < count ($vars); ++$i) { ?>
		<tr>
		<td><? echo ($vars[$i] . "<br>(" . $labels[$i] . ")"); ?></td>
	<? for ($j = 0; $j < count ($vars); ++$j) { ?>
		<td><? echo ($matrix[$i+1][$j]); ?></td>
	<?}?>
		</tr>
<?}?>
</table><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp)");
	}
?>
