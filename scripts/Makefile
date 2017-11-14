install : run_vsa  safepace_retrieve_radar.php  safepace_set_speed.php  VSA.html check_vsa_data_feed.sh
	cp -p run_vsa  safepace_retrieve_radar.php  safepace_set_speed.php  VSA.html vsamap.html check_vsa_data_feed.sh /var/www/html/VSA/scripts
	sudo cp -p run_vsa kill_vsa /usr/local/bin
	crontab -l >crontab.txt
