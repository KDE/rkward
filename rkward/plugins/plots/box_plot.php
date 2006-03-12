<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.tmp.x <- boxplot (<? getRK ("x"); ?>, notch = <? getRK ("notch") ?>, outline = <? getRK("outline")?>, horizontal = <? getRK("orientation") ?>)
<?
	}
	
	function printout () {
	$xlabel = getRK_val ("x.label")

?>
rk.graph.on()
cat ("<h1>Boxplot</h1>")
cat ("<h2><? getRK (x.label); ?></h2>")
cat ("<table align="center" frame="above">
  <tbody>
    <tr>
      <td>cat (deparse (rk.tmp.x))</td>
    </tr>
  </tbody>
</table>")

<?
	}
	
	function cleanup () {
?>rm (rk.tmp.x)
<?
	}
?>
