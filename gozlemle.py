#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
Created on 18.11.2016

@author: Ugur Akin
'''

from __future__ import division
import time
import os
import urllib2

def get_web(symbol, data_type):
	attempts = 0
	while attempts < 5:
		req = urllib2.Request('http://finance.yahoo.com/d/quotes.csv?s="{0}"&f={1}'.format(symbol,data_type))
		try:
			response = urllib2.urlopen(req)	
		except urllib2.URLError as e:
			attempts += 1
		else:
			# everything is fine
			return response.read()
	#and here all attempts consumed, return a failure
	return None

def html_text(text, color):
	return """<b><font color="{0}">{1}</b></font>""".format(color, text)
	
def hesapla( symbol, report_file, token_file_writer, my_token_file):
	#init return variable (0: nothing to report, 1: report required)
	report_required=0
	#today's date as YY-MM-DD
	today=time.strftime("%Y-%m-%d")
		
	##############################################
	#START ONLY IF TRADING FOR TODAY HAS STARTED
	##############################################
	
	#example output: "11/14/2016"
	trade_date = get_web(symbol, "d1")
	#check whether empty=web failure
	if trade_date is None:
		report_file.write(time.strftime("%H:%M") + ": olay var. wget hatasi 1. " + symbol + "<br>\n")
		#report not required
		return report_required
	
	#remove the surrounding quotes and the last carriage return
	trade_date = trade_date[1:-2]
	
	#put a "-" to prevent a leading zero
	today_reformatted=time.strftime("%-m/%-d/%Y")

	if(today_reformatted != trade_date):
		#new trading day has not begun yet, return with the initial value of report (=0 --> no report)
		return report_required
	
	##############################################
	#/START ONLY FIF TRADING FOR TODAY HAS STARTED
	##############################################
	
	#get daily low
	low = get_web(symbol, "g")
	#check whether empty=web failure
	if low is None:
		report_file.write(time.strftime("%H:%M") + ": olay var. wget hatasi 2. " + symbol + "<br>\n")
		#report not required
		return report_required
	
	#get daily high
	high = get_web(symbol, "h")
	#check whether empty=web failure
	if high is None:
		report_file.write(time.strftime("%H:%M") + ": olay var. wget hatasi 3. " + symbol + "<br>\n")
		#report not required
		return report_required

	#get current
	current = get_web(symbol, "l1")
	#check whether empty=web failure
	if current is None:
		report_file.write(time.strftime("%H:%M") + ": olay var. wget hatasi 4. " + symbol + "<br>\n")
		#report not required
		return report_required
		
	#get previous closure
	yesterday = get_web(symbol, "p")
	#check whether empty=web failure
	if yesterday is None:
		report_file.write(time.strftime("%H:%M") + ": olay var. wget hatasi 5. " + symbol + "<br>\n")
		#report not required
		return report_required

	#check whether any of the results is N/A
	if 'N/A\n' in {low, high, current, yesterday}:
		#missing data thus report not required
		return report_required
	
	#now it is safe to convert everything into float
	low=float(low)
	high=float(high)
	current=float(current)
	yesterday=float(yesterday)
	
	#calculate spread and format it to get the leading zero
	spread_per=float("{0:.2f}".format((high - low)*100/low))
	
	#calculate the percent of current to daily low and daily high and format it to get the leading zero
	low_per=float("{0:.2f}".format((current - low)*100/low))
	high_per=float("{0:.2f}".format(spread_per - (current - low)*100/low))

	#calculate performance to yesterday and format it to get the leading zero
	change_per=float("{0:.2f}".format((current - yesterday)*100/yesterday))
	
	
	#check whether the lower limits are reached and only if it was not reported before
	if(spread_per > 1.5 and low_per < 0.5 and not today+symbol+"L" in open(my_token_file,"r").read()):
		report_file.write("""{0}: ---{1}: |Low-{2}-Cur----{3}%----High| Spread: {4}%, Current: {5}<br>\n""".format(time.strftime("%H:%M"), symbol, html_text(str(low_per)+"%", "red"), high_per, spread_per,current))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"L"+"\n")
		report_required=1
	
	#check whether the high limits are reached and only if it was not reported before
	if(spread_per > 1.5 and high_per < 0.5 and not today+symbol+"H" in open(my_token_file,"r").read()):
		report_file.write("""{0}: +++{1}: |Low----{2}%----Cur-{3}-High| Spread: {4}%, Current: {5}<br>\n""".format(time.strftime("%H:%M"), symbol, low_per,  html_text(str(high_per)+"%", "forestgreen"), spread_per,current))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"H"+"\n")
		report_required=1

	#check whether perfo to yesterday is very good and only if it was not reported before
	if(change_per > 1.9 and not today+symbol+"CH" in open(my_token_file,"r").read()):
		report_file.write("""{0}: +++{1}: Big change to yesterday: {2}, Current: {3}<br>\n""".format(time.strftime("%H:%M"), symbol, html_text(str(change_per)+"%", "forestgreen"), current))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"CH"+"\n")
		report_required=1

	#check whether perfo to yesterday is very good and only if it was not reported before
	if(change_per < -1.9 and not today+symbol+"CL" in open(my_token_file,"r").read()):
		report_file.write("""{0}: ---{1}: Big change to yesterday: {2}, Current: {3}<br>\n""".format(time.strftime("%H:%M"), symbol, html_text(str(change_per)+"%", "red"), current))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"CL"+"\n")
		report_required=1


	####################################
	#SPECIFIC CHECKS
	####################################

	#check specific properties
	if(symbol=='EVK.DE' and current < 27.0 and not today+symbol+"S1" in open(my_token_file,"r").read()):
		report_file.write("""{0}: ---{1}: Low: {2}<br>\n""".format(time.strftime("%H:%M"), symbol, html_text(str(current), "red")))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"S1"+"\n")
		report_required=1

	#check specific properties
	if(symbol=='EVK.DE' and current > 28.00 and not today+symbol+"S2" in open(my_token_file,"r").read()):
		report_file.write("""{0}: +++{1}: High: {2}<br>\n""".format(time.strftime("%H:%M"), symbol, html_text(str(current), "forestgreen")))
		#update the token file to prevent numerous reporting on a single day
		token_file_writer.write(today+symbol+"S2"+"\n")
		report_required=1

	#tell the main program that the stdout has to be reported (=1) or not (=0)   
	return report_required  
	
####################################
#END OF FUNCTION DEFINITIONS
####################################	

#list of symbols to be processed
#symbol_list= ['PFV.DE', 'NOVC.DE', 'AIR.DE', 'XAUEUR=X', 'JEN.DE', 'BOSS.DE', 'ALV.DE', 'DAI.DE', 'WAF.DE']
#symbol_list= ['PFV.DE', 'NOVC.DE', 'AIR.DE', 'BOSS.DE', 'ALV.DE', 'DAI.DE']
symbol_list=['EVK.DE', 'ARL.DE' , 'AR4.DE' , 'ACX.DE']  
#use Tmpfs and iventually /etc/fstab to write to memory instead of sd card to extend its life
	
#Do we need to report anything?
report_required=0
	
## dd/mm/yyyy format
my_report_file="/home/pi/virtual_drv_10MB/rapor_"+time.strftime("%Y-%m-%d")

## dd/mm/yyyy format
my_token_file="/home/pi/virtual_drv_10MB/token_"+time.strftime("%Y-%m-%d")
#create token file for today if not existing
#change this part to "if" in order to prevent continuous writing to sd card
#in the current version it is running on virtual drive so no issue
token_file_writer=open(my_token_file, "a")

#is the report file for today alrady existing?
if(not os.path.isfile(my_report_file)):
	#start a new report for emails
	report_file=open(my_report_file, "a")
	report_file.write("MIME-Version: 1.0\n")
	report_file.write("Content-Type: text/html\n")
	report_file.write("Subject: [privat] Rapor var...\n\n")
	#get the external ip
	ext_ip = urllib2.urlopen('http://ip.42.pl/raw').read()
	report_file.write("My ip: %s <br><br>\n" % ext_ip)
	report_file.write("EVK.DE 30.79 <br>\n ARL.DE 36.75 <br>\n AR4.DE 42.95 <br>\n ACX.DE 112 <br><br>\n\n")
else: report_file=open(my_report_file, "a")


for symbol in symbol_list:
	response=hesapla(symbol, report_file, token_file_writer, my_token_file)
	report_required=report_required+response

#close and flush files before trying to send emails, otherwise you will find no content to send email :)
report_file.flush()
token_file_writer.flush()
os.fsync(report_file.fileno())
os.fsync(token_file_writer.fileno())
report_file.close
token_file_writer.close

#check whether we have something to report per email (>0)
if(report_required>0):
	os.system('cat {0} | /usr/sbin/sendmail ugur.akin@airbus.com ugur.akin@gmail.com'.format(my_report_file))
	print(time.strftime("%H:%M") + ": sending report per email.")
	#cat $report_file | msmtp -a default ugur.akin@airbus.com ugur.akin@gmail.com

