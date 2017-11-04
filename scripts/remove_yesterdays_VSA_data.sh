#!/bin/bash 

YESTERDAY_FILE_PREFIX=`date -d 'yesterday' +datafeed-%Y-%m-%d`

sudo rm -f /var/www/html/VSA/data/$YESTERDAY_FILE_PREFIX"-0"*
sudo rm -f /var/www/html/VSA/data/$YESTERDAY_FILE_PREFIX"-1"*
sudo rm -f /var/www/html/VSA/data/$YESTERDAY_FILE_PREFIX"-2"*
