install : run_vsa  safepace_retrieve_radar.php  safepace_set_speed.php  VSA.html check_vsa_data_feed.sh
	cp -p run_vsa  safepace_retrieve_radar.php  safepace_set_speed.php  check_vsa_data_feed.sh /var/www/html/VSA/scripts
#	The following copy is done from the operating directory to here because VSA.html and vsamap.html are modified and 
#	tested in the operating directory
	cp -p /var/www/html/VSA/scripts/VSA.html /var/www/html/VSA/scripts/vsamap.html .
	sudo cp -p run_vsa kill_vsa /usr/local/bin
	crontab -l >crontab.txt
