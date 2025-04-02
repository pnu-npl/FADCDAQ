#!usr/bin/perl -w

$Sid = 1;
$RunNumConverter=20032;

$InputDir="data_20241113";

for($ifile=0; $ifile<1; $ifile++)
{
	$InputFileName = sprintf("%s/jbnu_daq_%d_%d.dat", $InputDir, $Sid, $ifile);
	$OutputFileName = sprintf("%s/jbnu_daq_%d_%05d.dat", $InputDir, $Sid, $RunNumConverter+$ifile);

	system"mv ${InputFileName} ${OutputFileName}"
}
