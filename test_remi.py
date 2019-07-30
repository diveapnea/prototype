#!/usr/bin/env python
# -*- coding: utf-8 -*-



"""
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
"""

""" This simple example shows how to display a matplotlib plot image.
    The MatplotImage gets addressed by url requests that points to 
     a specific method. The displayed image url points to "get_image_data" 
    Passing an additional parameter "update_index" we inform the browser 
     about an image change so forcing the image update.
"""

import io
import datetime
import time
import threading

import remi.gui as gui
from remi import start, App

from matplotlib.figure import Figure
from matplotlib.backends.backend_agg import FigureCanvasAgg
import matplotlib.dates

from subprocess import call


def update_all_data():
	data_adc1 = []
	data_adc2 = []
	data_adc3 = []
	data_vcc = []
	data_tiny_temp = []
	data_raspi_temp = []
	data_x = []
	with open("/home/pi/data.log") as f:
		for line in f:
			if len(line.split()) >= 2:
				#take year and time and convert to plot-able format
				#0:date		1:time	 2:adc1 3:adc2 4:adc3 5:vcc  6:t_attiny 7:t_raspi
				#28.07.2019 00:08:22 0.000V 0.100V 1.550V 3.390V +22degC +55degC
				data_x.append(matplotlib.dates.datestr2num(''.join([line.split()[0], ' ', line.split()[1]])))
				#retrieve the voltage value
				data_adc1.append(float(line.split()[2].replace('V','')))
				data_adc2.append(float(line.split()[3].replace('V','')))
				data_adc3.append(float(line.split()[4].replace('V','')))
				data_vcc.append(float(line.split()[5].replace('V','')))
				data_tiny_temp.append(int(line.split()[6].replace('degC','')))
				data_raspi_temp.append(int(line.split()[7].replace('degC','')))				
	return data_x, data_adc1, data_adc2, data_adc3, data_vcc, data_tiny_temp, data_raspi_temp

class MatplotImage(gui.Image):
    ax = None

    def __init__(self, **kwargs):
        super(MatplotImage, self).__init__("/%s/get_image_data?update_index=0" % id(self), **kwargs)
        self._buf = None
        self._buflock = threading.Lock()

        self._fig = Figure(figsize=(6, 5)) #widthxheight      #figsize=(6, 5)  
        self.ax = self._fig.add_subplot(111)

        self.redraw()

    def redraw(self):
        canv = FigureCanvasAgg(self._fig)
        buf = io.BytesIO()
        canv.print_figure(buf, format='png')
        with self._buflock:
            if self._buf is not None:
                self._buf.close()
            self._buf = buf

        i = int(time.time() * 1e6)
        self.attributes['src'] = "/%s/get_image_data?update_index=%d" % (id(self), i)
        
        self._fig.tight_layout()
        super(MatplotImage, self).redraw()

    def get_image_data(self, update_index):
        with self._buflock:
            if self._buf is None:
                return None
            self._buf.seek(0)
            data = self._buf.read()

        return [data, {'Content-type': 'image/png'}]


class MyApp(App):
    def __init__(self, *args):
        super(MyApp, self).__init__(*args)


    def main(self):
        wid = gui.VBox(width=320, height=700, margin='0px auto')
        #wid = gui.VBox(width='100%', height='100%', margin='0px auto')
        wid.style['text-align'] = 'center'
        
        horizontalContainer_check = gui.Widget(width='100%',  height='5%', layout_orientation=gui.Widget.LAYOUT_HORIZONTAL, margin='1px', style={'display': 'block', 'overflow': 'auto'})
        horizontalContainer = gui.Widget(width='100%', height='5%', layout_orientation=gui.Widget.LAYOUT_HORIZONTAL, margin='1px', style={'display': 'block', 'overflow': 'auto'})
        
        
        self.bt = gui.Button('Update', width='20%', height='100%') #stand alone
        self.bt.style['margin'] = '0px'
        self.bt.onclick.do(self.on_button_pressed)
        
        self.bt_off = gui.Button('Shutdown', width='30%', height='5%') #in the container
        self.bt_off.style['margin'] = '0px'
        self.bt_off.onclick.do(self.on_button_pressed_off)
        
        self.chck_all = gui.CheckBoxLabel(label = 'Show all', checked = True, width='25%', height='5%', margin='0px')
        self.chck_all.onchange.do(self.on_check_change)
        
        self.chck1 = gui.CheckBoxLabel(label = 'adc1', checked = True, width='25%', height='100%', margin='px')
        self.chck1.onchange.do(self.on_check_change)
        self.chck2 = gui.CheckBoxLabel(label = 'adc2', checked = True, width='25%', height='100%', margin='0px')
        self.chck2.onchange.do(self.on_check_change)
        self.chck3 = gui.CheckBoxLabel(label = 'adc3', checked = True, width='25%', height='100%', margin='0px')
        self.chck3.onchange.do(self.on_check_change)
        self.chck_vcc = gui.CheckBoxLabel(label = 'vcc', checked = True, width='25%', height='100%', margin='0px')
        self.chck_vcc.onchange.do(self.on_check_change)

        
        self.date = gui.Label('', width='45%', height='100%', margin='1px')
        
        self.data_x, self.data_adc1, self.data_adc2, self.data_adc3,  \
        	self.data_vcc, self.data_tiny_temp, self.data_raspi_temp = update_all_data()

        
        self.mpl = MatplotImage(width='100%', height='40%')
        self.mpl.style['margin'] = '0px'
        self.mpl.ax.set_title("Voltage [V]")
        self.mpl.ax.grid(True)
        if self.chck1.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc1, label='adc1')
        if self.chck2.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc2, label='adc2')
        if self.chck3.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc3, label='adc3')
        if self.chck_vcc.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_vcc, label='vcc')
        #bbox_to_anchor x0, y0, width, height
        #locs = ['upper right', 'lower left', 'center left', 'lower center', 'center','right']   
        self.mpl.ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), fontsize='small' ,loc=0, ncol=4, mode="expand", borderaxespad=0.)          
        self.mpl.ax.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        for label in self.mpl.ax.xaxis.get_ticklabels():
        	label.set_rotation(45)        
        self.mpl.redraw()
        
        self.mpl_t = MatplotImage(width='100%', height='40%')
        self.mpl_t.style['margin'] = '0px'
        self.mpl_t.ax.set_title("Temp [degC]")
        self.mpl_t.ax.grid(True)
        self.mpl_t.ax.plot(self.data_x, self.data_tiny_temp, label='attiny85')
        self.mpl_t.ax.plot(self.data_x, self.data_raspi_temp, label='Raspberry')
        #bbox_to_anchor x0, y0, width, height
        #locs = ['upper right', 'lower left', 'center left', 'lower center', 'center','right']  
        self.mpl_t.ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), fontsize='small', loc=0, ncol=2, mode="expand", borderaxespad=0.)           
        self.mpl_t.ax.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        for label in self.mpl_t.ax.xaxis.get_ticklabels():
        	label.set_rotation(45)        
        self.mpl_t.redraw()
        
        
        self.shutdown = 0
		
        wid.append(self.bt_off)
        wid.append(horizontalContainer_check)
        horizontalContainer_check.append(self.chck1)
        horizontalContainer_check.append(self.chck2)
        horizontalContainer_check.append(self.chck3)
        horizontalContainer_check.append(self.chck_vcc)
        wid.append(self.mpl)
        wid.append(self.mpl_t)
        wid.append(horizontalContainer)
        horizontalContainer.append(self.chck_all)
        horizontalContainer.append(self.bt)
        horizontalContainer.append(self.date)

        return wid

    def onload(self, emitter):
        self.on_button_pressed('')
        self.update_datetime()

    def on_button_pressed(self, widget):
        self.data_x, self.data_adc1, self.data_adc2, self.data_adc3,  \
        	self.data_vcc, self.data_tiny_temp, self.data_raspi_temp = update_all_data()

    	if not self.chck_all.get_value():
    		self.data_x = self.data_x[-24*6:] #one day with 6 samples per hour
    		self.data_adc1 = self.data_adc1[-24*6:] #one day with 6 samples per hour
    		self.data_adc2 = self.data_adc2[-24*6:] #one day with 6 samples per hour
    		self.data_adc3 = self.data_adc3[-24*6:] #one day with 6 samples per hour
     		self.data_vcc = self.data_vcc[-24*6:] #one day with 6 samples per hour
    		self.data_tiny_temp = self.data_tiny_temp[-24*6:] #one day with 6 samples per hour
    		self.data_raspi_temp = self.data_raspi_temp[-24*6:] #one day with 6 samples per hour



        self.mpl.ax.cla()
        self.mpl.ax.set_title("Voltage [V]")
        self.mpl.ax.grid(True)
        if self.chck1.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc1, label='adc1')
        if self.chck2.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc2, label='adc2')
        if self.chck3.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_adc3, label='adc3')
        if self.chck_vcc.get_value():
        	self.mpl.ax.plot(self.data_x, self.data_vcc, label='vcc')
        #bbox_to_anchor x0, y0, width, height
        #locs = ['upper right', 'lower left', 'center left', 'lower center', 'center','right']   
        self.mpl.ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), fontsize='small', loc=0, ncol=4, mode="expand", borderaxespad=0.)          
        self.mpl.ax.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        for label in self.mpl.ax.xaxis.get_ticklabels():
        	label.set_rotation(45)        
        self.mpl.redraw()

        self.mpl_t.ax.cla()
        self.mpl_t.ax.set_title("Temp [degC]")
        self.mpl_t.ax.grid(True)
        self.mpl_t.ax.plot(self.data_x, self.data_tiny_temp, label='attiny85')
        self.mpl_t.ax.plot(self.data_x, self.data_raspi_temp, label='Raspberry')
        #bbox_to_anchor x0, y0, width, height 
        #locs = ['upper right', 'lower left', 'center left', 'lower center', 'center','right']
        self.mpl_t.ax.legend(bbox_to_anchor=(0., 1.02, 1., .102), fontsize='small', loc=0, ncol=2, mode="expand", borderaxespad=0.)           
        self.mpl_t.ax.xaxis.set_major_formatter(matplotlib.dates.DateFormatter('%H:%M'))
        for label in self.mpl_t.ax.xaxis.get_ticklabels():
        	label.set_rotation(45)        
        self.mpl_t.redraw()
         
        self.shutdown = 0
        self.bt_off.set_text('Shutdown')
        self.update_datetime()
        
    def on_check_change(self, widget, newValue): 
    	self.on_button_pressed(widget)

    def on_button_pressed_off(self, widget):
    	if self.shutdown == 0:
    		self.bt_off.set_text('Sure?')
    		self.shutdown = 1
    	else:
    		self.bt_off.set_text('Shutting down...')
    		call("sudo nohup shutdown -h now >/dev/null 2>&1", shell=True)

    def update_datetime(self):
        now = datetime.datetime.now()
        self.date.set_text(now.strftime("%H:%M:%S %d.%m.%Y"))    
        

if __name__ == "__main__":
    start(MyApp, address='0.0.0.0', port=8081)