<?php
include("ProgramResults.php");
include("NightlyTester.php");

$machine_id = $HTTP_GET_VARS['machine'];
$night_id = $HTTP_GET_VARS['night'];

if(!$machine_id || !$night_id){
	print "Error, incorrect URL for test.php!\n";
	die();
}


$mysql_link = mysql_connect("127.0.0.1","llvm","ll2002vm");
mysql_select_db("nightlytestresults");

$machine_query = mysql_query("SELECT * FROM machine WHERE id=$machine_id") or die (mysql_error());
$row = mysql_fetch_array($machine_query);
mysql_free_result($machine_query);
$today_query = mysql_query("SELECT * FROM night WHERE id=$night_id") or die (mysql_error());
$today_row = mysql_fetch_array($today_query);
mysql_free_result($today_query);
$cur_date=$today_row['added'];

$today_query = mysql_query("SELECT * FROM night WHERE machine=$machine_id and added<\"$cur_date\" order by added desc") or die (mysql_error());
$yesterday_row = mysql_fetch_array($today_query);
mysql_free_result($today_query);


?>

<html>
<head>
<title>LLVM Nightly Test Results For <?php print $cur_date; ?></title>
<STYLE TYPE="text/css">
<!--
  @import url(style.css);
-->
</STYLE>

<script type="text/javascript" src="sorttable.js"></script>
<script language="javascript">
function toggleLayer(whichLayer)
{
if (document.getElementById)
{
// this is the way the standards work
var style2 = document.getElementById(whichLayer).style;
style2.display = style2.display? "":"none";
var link  = document.getElementById(whichLayer+"_").innerHTML;
if(link.indexOf("(+)") >= 0){
      document.getElementById(whichLayer+"_").innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.getElementById(whichLayer+"_").innerHTML="(+)"+link.substring(3,link.length);
}

}//end if
else if (document.all)
{
// this is the way old msie versions work
var style2 = document.all[whichLayer].style;
style2.display = style2.display? "":"none";
var link  = document.all[wwhichLayer+"_"].innerHTML;
if(link.indexOf("(+)") >= 0){
      document.all[whichLayer+"_"].innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.all[whichLayer+"_"].innerHTML="(+)"+link.substring(3,link.length);
}

}
else if (document.layers)
{
// this is the way nn4 works
var style2 = document.layers[whichLayer].style;
style2.display = style2.display? "":"none";
var link  = document.layers[whichLayer+"_"].innerHTML;
if(link.indexOf("(+)") >= 0){
      document.layers[whichLayer+"_"].innerHTML="(-)"+link.substring(3,link.length);
}
else{
      document.layers[whichLayer+"_"].innerHTML="(+)"+link.substring(3,link.length);
}

}

}//end function
</script>

</head>
<body>

<center><font size=+3 face=Verdana><b>LLVM Nightly Test Results For <?php print $cur_date; ?></b></font></center><br>

<table cellspacing=0 cellpadding=0 border=0>
	<tr>
		<td valign=top>
			<? 
			$machine = $HTTP_GET_VARS['machine'];
			$night = $HTTP_GET_VARS['night'];
			include 'sidebar.php'; 
			?>			
		</td>
		<td>
<?php

/*****************************************************
 *
 * Printing machine information
 *
 ******************************************************/
print "<table border=1 cellpadding=0 cellspacing=0>\n";
print "<tr>\n";
print "<td><b>Nickname:</b></td>";
print "<td>{$row['nickname']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>uname:</b></td>";
print "<td>{$row['uname']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Hardware:</b></td>";
print "<td>{$row['hardware']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>OS:</b></td>";
print "<td>{$row['os']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Hostname:</b></td>";
print "<td>{$row['name']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>GCC:</b></td>";
print "<td>{$row['gcc']}</td>\n";
print "</tr>\n";
print "<tr>\n";
print "<td><b>Machine ID:</b></td>";
print "<td>{$row['id']}</td>\n";
print "</tr>\n";
print "</table>\n<br>\n";

/*****************************************************
 *
 * Printing the times table
 *
 ******************************************************/
$previous_query = mysql_query("SELECT * FROM night WHERE \"$cur_date\" > added and machine=$machine_id ORDER BY added DESC") or die (mysql_error());

if(strpos($today_row['buildstatus'], "OK")!=FALSE){
	$disp="";
	$sign="(+)";
}
else{
	$disp="none";
	$sign="(-)";
}
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('buildStatus');\", id=\"buildStatus_\">$sign Build Status</a></font>\n";
print "<div id=\"buildStatus\" style=\"display: $disp;\" class=\"hideable\">\n";
print "<h3><u>Build Status </u></h3></p>";
print "<font color=red>{$today_row['buildstatus']}</font><br>\n";
print "</div><br><br>\n";


/*****************************************************
 *
 * Printing changes in test suite
 *
 ******************************************************/
$new_tests=preg_replace("/\n/","<br>\n",$today_row['new_tests']);
$removed_tests=preg_replace("/\n/","<br>\n",$today_row['removed_tests']);
$newly_passing_tests=preg_replace("/\n/","<br>\n",$today_row['newly_passing_tests']);
$newly_failing_tests=preg_replace("/\n/","<br>\n",$today_row['newly_failing_tests']);

if((strpos($new_tests, "none")!=FALSE &&
   strpos($removed_tests, "none")!=FALSE &&
   strpos($newly_passing_tests, "none")!=FALSE &&
   strpos($newly_failing_tests, "none")!=FALSE ) ||
   (strcmp($new_tests, "")==0 &&
   strcmp($removed_tests, "")==0 &&
   strcmp($newly_passing_tests, "")==0 &&
   strcmp($newly_failing_tests, "")==0)){
        $disp="none";
        $sign="(-)";
}
else{
	$disp="";
	$sign="(+)";
}
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('testSuite');\", id=\"testSuite_\">$sign Test Suite Changes</a></font>\n";
print "<div id=\"testSuite\" style=\"display: $disp;\" class=\"hideable\">\n";
print"<h3><u>Test suite changes:</u></h3>\n";
print"<b>New tests:</b><br>\n";
print "$new_tests<br><br>\n";
print"<b>Removed tests</b><br>\n";
print "$removed_tests<br><br>\n";
print"<b>Newly passing tests:</b><br>\n";
print "$newly_passing_tests<br><br>\n";
print"<b>Newly failing tests:</b><br>\n";
print "$newly_failing_tests<br><br>\n";
print "</div><br><br>\n";

/*****************************************************
 *
 * Dejagnu result go here
 *
 ******************************************************/
$delta_exppass = $today_row['teststats_exppass']-$yesterday_row['teststats_exppass'];
$delta_expfail = $today_row['teststats_expfail']-$yesterday_row['teststats_expfail'];
$delta_unexpfail = $today_row['teststats_unexpfail']-$yesterday_row['teststats_unexpfail'];

if($delta_exppass==0 && $delta_expfail==0 && $delta_unexpfail==0){
        $disp="none";
        $sign="(-)";
}
else{
        $disp="";
        $sign="(+)";
}


if(isset($today_row['teststats_exppass'])){
        $exp_pass=$today_row['teststats_exppass'];
}
else{
        $exp_pass=0;
}
if(isset($today_row['teststats_unexpfail'])){
        $unexp_fail=$today_row['teststats_unexpfail'];
}
else{
        $unexp_fail=0;
}
if(isset($today_row['teststats_expfail'])){
        $exp_fail=$today_row['teststats_expfail'];
}
else{
        $exp_fail=0;

}

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('dejagnuTests');\", id=\"deja\
gnuTests_\">$sign Dejagnu Tests</a></font>\n";
print "<div id=\"dejagnuTests\" style=\"display: $disp;\" class=\"hideable\">\n";

print"<h3><u>Dejagnu tests:</u></h3><br>\n";
print "<table>\n";
print "\t<tr>\n";
print "\t\t<td></td><td># of tests</td><td>Change from yesterday</td>\n";
print "\t</tr>\n";
$delta = $today_row['teststats_exppass']-$yesterday_row['teststats_exppass'];
print "\t\t<td>Expected Passes:</td><td align=center>$exp_pass</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
$delta = $today_row['teststats_unexpfail']-$yesterday_row['teststats_unexpfail'];
print "\t\t<td>Unexpected Failures:</td><td align=center>$unexp_fail</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
$delta = $today_row['teststats_expfail']-$yesterday_row['teststats_expfail'];
print "\t\t<td>Expected Failures:</td><td align=center>$exp_fail</td><td align=center>$delta</td>\n";
print "\t</tr>\n";
print "</table><br><br>\n";

print"<a name=\"unexpfail_tests\"><b>Unexpected test failures:</b></a><br>\n";
$unexpfail_tests=preg_replace("/\n/","<br>\n",$today_row['unexpfail_tests']);
print "$unexpfail_tests<br><br>\n";

print "</div><br><br>\n";

/*****************************************************
 *
 * Printing warning information
 *
 ******************************************************/
if((strpos($today_row['warnings_added'], "none")==FALSE &&
   strpos($today_row['warnings_removed'], "none")==FALSE) &&
   (strcmp($today_row['warnings_added'], "")!=0 &&
   strcmp($today_row['warnings_removed'], "")!=0)){
        $disp=" ";
        $sign="(+)";
}
else{
	$disp="none";
	$sign="(-)";
}
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('warningsChanges');\", id=\"warningsChanges_\">$sign Warning Information</a></font>\n";
print "<div id=\"warningsChanges\" style=\"display: $disp;\" class=\"hideable\">\n";
print"<h3><u>Changes to warnings during the build:</u></h3>\n";
print"<b>New Warnings:</b><br>\n";
print "{$today_row['warnings_added']}<br><br>\n";
print"<b>Removed Warnings:</b><br>\n";
print "{$today_row['warnings_removed']}<br>\n";
print"<a name=\"warnings\"><h3><u>Warnings during the build:</u></h3></a><tt>{$today_row['warnings']}</tt><br>\n";
print "</div><br><br>\n";


/*****************************************************
 *
 * Printing execution
 *
 ******************************************************/

print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('executionTimes');\", id=\"executionTimes_\">(+) Execution Times</a></font>\n";
print "<div id=\"executionTimes\" style=\"display:;\" class=\"hideable\">\n";
print"<h3><u>Execution times in seconds:</u></h3><br>\n";
print "<table border=1 cellpadding=0 cellspacing=0>\n";

print "\t<tr>\n";
print "\t\t<td></td>\n";
print "\t\t<td>CVS cpu</td>\n";
print "\t\t<td>CVS wall</td>\n";
print "\t\t<td>Configure cpu</td>\n";
print "\t\t<td>Configure wall</td>\n";
print "\t\t<td>Build cpu</td>\n";
print "\t\t<td>Build wall</td>\n";
print "\t\t<td>Dejagnu cpu</td>\n";
print "\t\t<td>Dejagnu wall</td>\n";
print "\t</tr>\n";

print "\t<tr>\n";
print "\t\t<td>$cur_date</td>\n";
print "\t\t<td>{$today_row['getcvstime_cpu']}</td>\n";
print "\t\t<td>{$today_row['getcvstime_wall']}</td>\n";
print "\t\t<td>{$today_row['configuretime_cpu']}</td>\n";
print "\t\t<td>{$today_row['configuretime_wall']}</td>\n";
print "\t\t<td>{$today_row['buildtime_cpu']}</td>\n";
print "\t\t<td>{$today_row['buildtime_wall']}</td>\n";
print "\t\t<td>{$today_row['dejagnutime_cpu']}</td>\n";
print "\t\t<td>{$today_row['dejagnutime_wall']}</td>\n";
print "\t</tr>\n";

if( $previous_row = mysql_fetch_array($previous_query) ){
	print "\t<tr>\n";
	print "\t\t<td>Previous nightly test ({$previous_row['added']})</td>\n";
	print "\t\t<td>{$previous_row['getcvstime_cpu']}</td>\n";
	print "\t\t<td>{$previous_row['getcvstime_wall']}</td>\n";
	print "\t\t<td>{$previous_row['configuretime_cpu']}</td>\n";
	print "\t\t<td>{$previous_row['configuretime_wall']}</td>\n";
	print "\t\t<td>{$previous_row['buildtime_cpu']}</td>\n";
	print "\t\t<td>{$previous_row['buildtime_wall']}</td>\n";
	print "\t\t<td>{$previous_row['dejagnutime_cpu']}</td>\n";
	print "\t\t<td>{$previous_row['dejagnutime_wall']}</td>\n";
	print "\t</tr>\n";


	print "\t<tr>\n";
	print "\t\t<td>% change</td>\n";
	
	if($previous_row['getcvstime_cpu']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['getcvstime_cpu'] - $previous_row['getcvstime_cpu'])/$previous_row['getcvstime_cpu']) * 100,2);	
		$color=DetermineColor($delta, "white");
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}	

	$color="white";
	if($previous_row['getcvstime_wall']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['getcvstime_wall'] - $previous_row['getcvstime_wall'])/$previous_row['getcvstime_wall']) * 100,2);
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}
	
	$color="white";
	if($previous_row['configuretime_cpu']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['configuretime_cpu'] - $previous_row['configuretime_cpu'])/$previous_row['configuretime_cpu']) * 100,2);
        	$color=DetermineColor($delta, "white");
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}
	
	$color="white";

	if($previous_row['configuretime_wall']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['configuretime_wall'] - $previous_row['configuretime_wall'])/$previous_row['configuretime_wall']) * 100,2);
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}

	$color="white";
	if($previous_row['buildtime_cpu']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['buildtime_cpu'] - $previous_row['buildtime_cpu'])/$previous_row['buildtime_cpu']) * 100,2);
	        $color=DetermineColor($delta, "white");
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}

	$color="white";
	if($previous_row['buildtime_wall']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['buildtime_wall'] - $previous_row['buildtime_wall'])/$previous_row['buildtime_wall']) * 100,2);
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}
	
	$color="white";
	if($previous_row['dejagnutime_cpu']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['dejagnutime_cpu'] - $previous_row['dejagnutime_cpu'])/$previous_row['dejagnutime_cpu']) * 100,2);
		$color=DetermineColor($delta, "white");
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}

	$color="white";
	if($previous_row['dejagnutime_wall']==0){
		print "\t\t<td>-</td>\n";
	}
	else{
		$delta = round((($today_row['dejagnutime_wall'] - $previous_row['dejagnutime_wall'])/$previous_row['dejagnutime_wall']) * 100,2);
		print "\t\t<td bgcolor=$color>$delta</td>\n";
	}
	
	print "\t</tr>\n";
}
mysql_free_result($previous_query);

print "</table>\n";

print "</div><br><br>\n";

/*****************************************************
 *
 * Printing CVS information
 *
 ******************************************************/
print "<font size=\"-1\"><a href=\"javascript://\"onclick=\"toggleLayer('CVSInformation');\", id=\"CVSInformation_\">(-) CVS information</a></font>\n";
print "<div id=\"CVSInformation\" style=\"display: none;\" class=\"hideable\">\n";

print"<h3><u>CVS information:</u></h3><br>\n";

$query=mysql_query("SELECT * FROM night WHERE id=$night_id") or die (mysql_error());
$row = mysql_fetch_array($query);
mysql_free_result($query);
$com_users = $row['cvs_usersadd'];
$co_users  = $row['cvs_usersco'];
$com_users = str_replace("\n","<br>",$com_users);
$co_users = str_replace("\n","<br>",$co_users);
print "<table border=1 cellspacing=0 cellpadding=0>\n";
print "\t<tr>\n";
print "\t\t<td>Users who commited</td><td>Users who checked out</td>\n";
print "\t</tr>\n";
print "\t<tr>\n";
print "\t\t<td valign=top>$com_users</td><td valign=top>$co_users</td>\n";
print "\t</tr>\n";
print "</table><br><br>\n";

print"<b>Added files:</b><br>\n";
$added_files  = $row['cvs_added'];
if(strcmp($added_files,"")!=0){
	$added_files = str_replace("\n","<br>",$added_files);
	print "<table>\n";
	print "\t<tr>\n";
	print "\t\t<td>$added_files</td>\n";
	print "\t</tr>\n";
	print "</table><br><br>\n";
}
else{
	print "No removed files<br><br>\n";
}

print"<b>Removed files:</b><br>\n";
$removed_files  = $row['cvs_removed'];
if(strcmp($removed_files,"")!=0){
	$removed_files = str_replace("\n","<br>",$removed_files);
	print "<table>\n";
	print "\t<tr>\n";
	print "\t\t<td>$removed_files</td>\n";
	print "\t</tr>\n";
	print "</table><br><br>\n";
}
else{
	print "No removed files<br><br>\n";
}

print"<b>Modified files:</b><br>\n";
$modified_files  = $row['cvs_modified'];
if(strcmp($modified_files,"")!=0){
	$modified_files = str_replace("\n","<br>",$modified_files);
	print "<table>\n";
	print "\t<tr>\n";
	print "\t\t<td>$modified_files</td>\n";
	print "\t</tr>\n";
	print "</table><br><br>\n";
}
else{
	print "No removed files<br><br>\n";
}
print "</div><br><br>\n";

/*****************************************************
 *
 * ending sidebar table here
 *
 ******************************************************/
print "</td></tr></table>\n";


/*****************************************************
 *
 * Test program statistics
 *
 ******************************************************/
print"<h3><u>Program tests:</u></h3><br>\n";

$today_results = GetDayResults($today_row['id'], $category_array, $mysql_link);
if(isset($yesterday_row['id'])){
	$yesterday_results = GetDayResults($yesterday_row['id'], $category_array, $mysql_link);
	$percent_difference = CalculateChangeBetweenDays($yesterday_results, $today_results,.2);	
}
/********************** external table **********************/
print "<form method=GET action=\"resultsgraph.php\">\n";
print "<input type=hidden name=machine value=\"$machine_id\">\n";
print "<input type=hidden name=night value=\"$night_id\">\n";
print "<input type=hidden name=end value=\"$cur_date\">\n";

print"<b>External tests:</b><br>\n";
print "<table border='0' cellspacing='0' cellpadding='2'><tr><td bgcolor=#000000>\n"; #creating the black border around the table 
print "<table class=\"sortable\" id=\"external_tests_\" border='1' cellspacing='0' cellpadding='0'>\n";
print "\t<tr bgcolor=#FFCC99>\n";	
print "\t\t<th>Program</th>\n";	
$index=0; //here to ensure we dont print %diff for GCC comparisons
foreach ($category_print_array as $x){
	print "\t\t<th>Today's $x</th>\n";
	if($index<10 && isset($percent_difference)){
		print "\t\t<th>% change in $x</th>\n";
	}
	$index++;
}
print "\t</tr>\n";	
print "\t<tr bgcolor=#FFCC99>\n";
print "\t\t<td></td>\n";
$index=0;	
foreach ($category_print_array as $x){
	if($index<10){
		print "\t\t<td colspan=\"2\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	else{
		print "\t\t<td colspan=\"1\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	$index++;
}
print "\t</tr>\n";	
$row_color=1;
$count=0;
foreach(array_keys($today_results) as $program){
	if(strcmp($today_results["$program"][0],"external")==0){
		if($row_color % 2 == 0){
			$def_color="white";
		}
		else{
			$def_color="#DDDDDD";
		}	
		print "\t<tr bgcolor='$def_color'>\n";		
		print "\t\t<td><input type=checkbox name=program[] multiple=\"multiple\" value=\"$program\">$program</td>\n";
		for($y=1; $y<sizeof($today_results["$program"]); $y++){
			print "\t\t<td>{$today_results["$program"][$y]}</td>\n";		
			if($y<11 && isset($percent_difference)){
				$delta=round($percent_difference["$program"][$y-1], 2);
				$color=DetermineColor($delta, $def_color);
				print "\t\t<td bgcolor=\"$color\">$delta</td>\n";	
			}
		} 
		print "\t</tr>\n";	
		$row_color++;
		if($row_color > 4){
			$row_color=1;
		}
		$count++;
	}//end if strcmp
}//end foreach
print "</table>\n";
print "</td></tr></table><br><br>\n"; #ending black border around table


/********************** Singlesource table **********************/

print"<b>Singlesource tests:</b><br>\n";
print "<table border='0' cellspacing='0' cellpadding='2'><tr><td bgcolor=#000000>\n"; #creating the black border around the table 
print "<table class=\"sortable\" id=\"singlesource_tests\" border='1' cellspacing='0' cellpadding='0'>\n";
print "\t<tr bgcolor=#FFCC99>\n";	
print "\t\t<th>Program</th>\n";	
$index=0; //here to ensure we dont print %diff for GCC comparisons
foreach ($category_print_array as $x){
	print "\t\t<th>Today's $x</th>\n";
	if($index<10 && isset($percent_difference)){
		print "\t\t<th>% change in $x</th>\n";
	}
	$index++;
}
print "\t</tr>\n";	
print "\t<tr bgcolor=#FFCC99>\n";
print "\t\t<td></td>\n";
$index=0;	
foreach ($category_print_array as $x){
	if($index<10){
		print "\t\t<td colspan=\"2\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	else{
		print "\t\t<td colspan=\"1\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	$index++;
}
print "\t</tr>\n";	
$row_color=1;
$count=0;
foreach(array_keys($today_results) as $program){
	if(strcmp($today_results["$program"][0],"singlesource")==0){
		if($row_color % 2 == 0){
			$def_color="white";
		}
		else{
			$def_color="#DDDDDD";
		}	
		print "\t<tr bgcolor='$def_color'>\n";		
		print "\t\t<td><input type=checkbox name=program[] multiple=\"multiple\" value=\"$program\">$program</td>\n";
		for($y=1; $y<sizeof($today_results["$program"]); $y++){
			print "\t\t<td>{$today_results["$program"][$y]}</td>\n";
			if($y<11 && isset($percent_difference)){
				$delta=round($percent_difference["$program"][$y-1], 2);
				$color=DetermineColor($delta, $def_color);
				print "\t\t<td bgcolor=\"$color\">$delta</td>\n";	
			}
		} 
		print "\t</tr>\n";	
		$row_color++;
		if($row_color > 4){
			$row_color=1;
		}
		$count++;
	}//end if strcmp
}//end foreach
print "</table>\n";
print "</td></tr></table><br><br>\n"; #ending black border around table


/********************** Multisource table **********************/

print"<b>Multisource tests:</b><br>\n";
print "<table border='0' cellspacing='0' cellpadding='2'><tr><td bgcolor=#000000>\n"; #creating the black border around the table 
print "<table class=\"sortable\" id=\"multisource_tests\" border='1' cellspacing='0' cellpadding='0'>\n";
print "\t<tr bgcolor=#FFCC99>\n";	
print "\t\t<th>Program</th>\n";	
$index=0; //here to ensure we dont print %diff for GCC comparisons
foreach ($category_print_array as $x){
	print "\t\t<th>Today's $x</th>\n";
	if($index<10 && isset($percent_difference)){
		print "\t\t<th>% change in $x</th>\n";
	}
	$index++;
}
print "\t</tr>\n";	
print "\t<tr bgcolor=#FFCC99>\n";
print "\t\t<td></td>\n";
$index=0;	
foreach ($category_print_array as $x){
	if($index<10){
		print "\t\t<td colspan=\"2\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	else{
		print "\t\t<td colspan=\"1\" align=center><input type=checkbox name=\"measure[]\" multiple=\"multiple\" value=\"$x\"></td>\n";
	}
	$index++;
}
print "\t</tr>\n";	
$row_color=1;
$count=0;
foreach(array_keys($today_results) as $program){
	if(strcmp($today_results["$program"][0],"multisource")==0){
		if($row_color % 2 == 0){
			$def_color="white";
		}
		else{
			$def_color="#DDDDDD";
		}	
		print "\t<tr bgcolor='$def_color'>\n";		
		print "\t\t<td><input type=checkbox name=program[] multiple=\"multiple\" value=\"$program\">$program</td>\n";
		for($y=1; $y<sizeof($today_results["$program"]); $y++){
			print "\t\t<td>{$today_results["$program"][$y]}</td>\n";
			if($y<11 && isset($percent_difference)){
				$delta=round($percent_difference["$program"][$y-1], 2);
				$color=DetermineColor($delta, $def_color);
				print "\t\t<td bgcolor=\"$color\">$delta</td>\n";	
			}
		} 
		print "\t</tr>\n";	
		$row_color++;
		if($row_color > 4){
			$row_color=1;
		}
		$count++;
	}//end if strcmp
}//end foreach
print "</table>\n";
print "</td></tr></table><br><br>\n"; #ending black border around table

print "<input type=submit name=action value=\"Compare values\"> | ";
print "<input type=reset>\n";
print "</form>\n";


mysql_close($mysql_link);
?>



</body>
</html>


