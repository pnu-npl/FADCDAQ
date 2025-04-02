#!usr/bin/perl -w

use Cwd;
use File::Copy;

# Make directories if there is no data dir
$current_dir = getcwd;
$dir_data=sprintf("%s/data", $current_dir);

$minRunNum=10000;
$maxRunNum=10001;

# Decoder version
# Empty	: Version made by Dr.Chong
# 2	: Version made by Prof.Sanghoon
$decoderVer="";

unless( -d $dir_data ){
	mkdir $dir_data or die "Cannot make data directory! :$!\n";
}

$dir_decoded = sprintf("${dir_data}/decoded");
unless( -d $dir_decoded){
	mkdir $dir_decoded or die "Cannot make decoded directory! :$!\n";
}

$dir_pdfs = sprintf("${dir_data}/pdfs${decoderVer}");
unless( -d $dir_pdfs){
	mkdir $dir_pdfs or die "Cannot make pdfs directory! :$!\n";
}

$dir_log = sprintf("${dir_data}/logs");
unless( -d $dir_log){
	mkdir $dir_log or die "Cannot make logs directory! :$!\n";
}

$src_decode="jbnu_daq_decode${decoderVer}.C";
$src_makepdf="jbnu_daq_draw.C";

# First, check if 
for( $irun=$minRunNum; $irun<$maxRunNum; $irun++)
{
	$datfile = sprintf("jbnu_daq_1_%05d.dat", $irun);
	$datfile_indir = sprintf("%s/%s", $dir_data, $datfile);

	$rootfile = sprintf("out_jbnu_daq_1_%05d.root", $irun);
	$rootfile_indir = sprintf("%s/%s", $dir_decoded, $rootfile);

	$pdffile = sprintf("EventByChannel_out_jbnu_daq_1_%d.pdf", $irun);
	$pdffile_indir = sprintf("%s/%s", $dir_pdfs, $pdffile);

	$logtext = sprintf("%s/setlog.txt", $current_dir);
	$logtext_output = sprintf("%s/setlog_1_%5d.txt", $dir_log, $irun);

	printf("${logtext}\n");
	printf("${logtext_output}\n");

	# Check if data were taken
	if( (! -f $datfile) and (! -f $datfile_indir) ){

		printf("No .datfile! Run and take data first!\n");
		next;

	}else{
		
		if( -f $datfile){
			system "mv ${datfile} ${dir_data}/${datfile}";	# Move .dat file into directory
		}

	}
	
	# Copy setlog and save
	#
	if( -f ${logtext_output} ){
		print("setlog.txt for this run already coppied!\n");
	} else {
		print("Copying current settings...");
		if(copy(${logtext}, ${logtext_output})){
			printf("Copying setlog.txt successful!\n");
		} else {
			printf("Copying setlog.txt failed!\n");
		}
	}

	# Run decoding if there are no decoded files
	if( ! -f $rootfile_indir ){
		
		system "cp $src_decode $dir_data";
		chdir $dir_data; 
		system "root -l -b -q '$src_decode(\"${datfile}\")'";
		system "rm $src_decode";
		system "mv $rootfile $rootfile_indir";
		print "Decoding ${irun} finished!\n";
		chdir $current_dir;

	} else {
		print "Decoding for ${irun} already done!\n";
	}

	# Draw pdf files if there are no pdf files
	if( ! -f $pdffile_indir){
		system "cp $src_makepdf $dir_pdfs";
		chdir $dir_pdfs;
		system "cp $rootfile_indir .";
		system "root -l -b -q '$src_makepdf(\"${rootfile}\", \"${logtext_output}\", true)'";
		system "rm $src_makepdf";
		system "rm $rootfile";
		print "Drawing pdf file for ${irun} finished!\n";
		chdir $current_dir;
	} else {
		print "Drawing pdf file for ${irun} already done!\n";
	}

}
