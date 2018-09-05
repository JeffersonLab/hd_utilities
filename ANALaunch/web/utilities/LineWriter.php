<?php
echo $_GET["jstr"];
$file='';
//echo "<br>" . $file;
$jsonOBJ=json_decode($_GET["jstr"],true);
//echo $jsonOBJ;
$Reaction=$jsonOBJ["Reaction"];
$Decays=$jsonOBJ["Decays"];
//echo "Reaction: ". $Reaction;
$B=$jsonOBJ["B"];
$F=$jsonOBJ["F"];
$T=$jsonOBJ["T"];
$U=$jsonOBJ["U"];
$Mlist=$jsonOBJ["Marray"];

$file=$file . $Reaction;
//echo "<br>" . $file;
foreach ($Decays as $dec)
{
    $file=$file . "_" . $dec;
}
//echo "<br>" . $file;
$file=$file . "-B" . $B;
$file=$file . "F" . $F;
$file=$file . "T" . $T;
$file=$file . "U" . $U;
//echo "<br>" . $file;
foreach ($Mlist as $M)
{
    $file=$file . "_M" . $$M;
}

echo "<br>" . $file;
$fp = fopen('/u/group/halld/www/halldweb/data/webdata/analysis/newlines/' . $file . '.json','w') or die('Cannot open file:  '.$file);
fwrite($fp,json_encode($_GET["jstr"]));
fclose($fp);

?>