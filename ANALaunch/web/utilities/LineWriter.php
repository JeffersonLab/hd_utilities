<?php
echo $_GET["jstr"];
echo $_GET["dataset"];
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
#$file=$file . "-B" . $B;
$file=$file . "-F" . $F;
#$file=$file . "T" . $T;
#$file=$file . "U" . $U;
//echo "<br>" . $file;
foreach ($Mlist as $M)
{
    $file=$file . "_M" . $M;
}
$newlB=true;
$newlT=true;
$newlU=true;
if(file_exists("/u/group/halld/www/halldweb/data/webdata/analysis/newlines/" . $_GET["dataset"] . '/' . $file . ".json"))
{
    $string = file_get_contents("/u/group/halld/www/halldweb/data/webdata/analysis/newlines/" . $_GET["dataset"] . '/' . $file . ".json");
    $json_fromF = json_decode($string, true);

    if(intval($B)<intval($json_fromF["B"]))
    {
        $B=$json_fromF["B"];
        $jsonOBJ["B"]=$B;
        $newlB=false;
    }
    

    if(intval($T)<intval($json_fromF["T"]))
    {
        $T=$json_fromF["T"];
        $jsonOBJ["T"]=$T;
        $newlT=false;
    }

    if(intval($U)<intval($json_fromF["U"]))
    {
        $U=$json_fromF["U"];
        $jsonOBJ["U"]=$U;
        $newlU=false;
    }

    

}
if($newlB || $newlT || $newlU)
{
    echo "<br>" . $file;
    $fp = fopen('/u/group/halld/www/halldweb/data/webdata/analysis/newlines/' . $_GET["dataset"] . '/' . $file . '.json','w') or die('Cannot open file:  '.$file);
    //fwrite($fp,"\"");
    fwrite($fp,json_encode($jsonOBJ));
    //fwrite($fp,"\"");
    fclose($fp);   
}


?>