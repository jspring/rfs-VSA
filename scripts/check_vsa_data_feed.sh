#!/bin/bash

cd /var/www/html/VSA/data
LASTFILE=`ls -t | head -1`
LASTFILETIMESTAMP=`stat -c %Y $LASTFILE`
CURRTIME=`date -u +%s`
TIMEDIFF=$(($CURRTIME - $LASTFILETIMESTAMP))

if [[ $TIMEDIFF -gt 60 ]]
then
	echo "To:jspring@berkeley.edu" >~/message.txt
	echo "CC:papowell@astart.com" >>~/message.txt
	echo "CC:anush.badii@dot.ca.gov" >>~/message.txt
	echo "Subject:VSA data feed status" >>~/message.txt
	echo  >>~/message.txt
	echo "Time now is: " `date` >>~/message.txt
	echo "Most recent file in data directory is: " >>~/message.txt
	ls -l $LASTFILE >>~/message.txt
	/usr/sbin/ssmtp -t <~/message.txt
fi
