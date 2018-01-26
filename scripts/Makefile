install : run_vsa  /var/www/html/VSA/scripts/safepace_retrieve_radar.php  /var/www/html/VSA/scripts/safepace_set_speed.php  /var/www/html/VSA/scripts/VSA.html /var/www/html/VSA/scripts/vsamap.html check_vsa_data_feed.sh /var/www/html/VSA/scripts/VSA_performance_json.php  /var/www/html/VSA/scripts/VSA_performance_plot.html
	cp -p run_vsa  check_vsa_data_feed.sh /var/www/html/VSA/scripts
#	The following copy is done from the operating directory to here because VSA.html, vsamap.html, 
#	safepace_retrieve_radar.php, and safepace_set_speed.php are modified and tested in the operating directory
	cp -p /var/www/html/VSA/scripts/VSA.html /var/www/html/VSA/scripts/vsamap.html .
	cp -p /var/www/html/VSA/scripts/safepace_retrieve_radar.php  /var/www/html/VSA/scripts/safepace_set_speed.php .
	cp -p /var/www/html/VSA/scripts/VSA_performance_json.php  /var/www/html/VSA/scripts/VSA_performance_plot.html .
	sudo cp -p run_vsa kill_vsa /usr/local/bin
	crontab -l >crontab.txt
