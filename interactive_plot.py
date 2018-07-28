"""
compute the mean and stddev of 100 data sets and plot mean vs stddev.
When you click on one of the mu, sigma points, plot the raw data from
the dataset that generated the mean and stddev
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import matplotlib.cm as cmx
import matplotlib.collections as collections


jet = cm = plt.get_cmap('jet') 

X = np.random.rand(100, 10)
xs = np.mean(X, axis=1)
ys = np.std(X, axis=1)
dataind = 0

f, axarr = plt.subplots(2, 1)
axarr[0].grid()
axarr[1].grid()

#calibrate colors with the actual values of the data
cNorm  = colors.Normalize(vmin=xs.min(), vmax=xs.max())
scalarMap = cmx.ScalarMappable(norm=cNorm, cmap=jet)

colorVal = scalarMap.to_rgba(xs[0])
line = axarr[0].scatter(xs, ys,s=100,alpha=0.5, c=xs, cmap='jet', picker=1) #picker tolerance of 1
line2 = axarr[1].scatter(np.arange(0,X[dataind].size),X[0],c=colorVal, s=100, picker=5)

annot = axarr[1].annotate("", xy=(0,0), xytext=(20,20),textcoords="offset points",
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
annot.set_visible(False)

def update_annot(ind):
	global colorVal
	pos = line2.get_offsets()[ind["ind"][0]]
	annot.xy = pos
	text = str(np.arange(0,X[dataind].size)[ind["ind"][0]]) + ' , ' + str(X[dataind][ind["ind"][0]])#"{}".format(" ".join(list(map(str,ind["ind"]))))
	annot.set_text(text)
	annot.get_bbox_patch().set_facecolor(colorVal)
	annot.get_bbox_patch().set_alpha(0.4)

def hover(event):
	vis = annot.get_visible()
	
	if event.inaxes == axarr[1]:
		cont, ind = line2.contains(event)		
		if cont:
			update_annot(ind)
			annot.set_visible(True)
			f.canvas.draw_idle()
		else:
			if vis:
				annot.set_visible(False)
				f.canvas.draw_idle()


def onpick(event):
	global line2
	global colorVal
	global dataind
	if event.artist == line:
		N = len(event.ind)
		if not N: return True

		for subplotnum, dataind in enumerate(event.ind):
			#comment out if you want to keep plots on top of each other
			axarr[1].cla()
			colorVal = scalarMap.to_rgba(xs[dataind])
			line2 = axarr[1].scatter(np.arange(0,X[dataind].size),X[dataind],c=colorVal, s=100, picker=5)
			axarr[1].text(0.05, 0.9, 'mu=%1.3f\nsigma=%1.3f'%(xs[dataind], ys[dataind]),
					transform=axarr[1].transAxes, va='top')
			axarr[1].set_ylim(-0.5, 1.5)
			axarr[1].set_title('mu=%1.3f sigma=%1.3f'%(xs[dataind], ys[dataind]))
			axarr[1].grid()			
		plt.draw()
		global annot
		annot = axarr[1].annotate("", xy=(0,0), xytext=(20,20),textcoords="offset points",
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
		annot.set_visible(False)
	elif event.artist == line2:
		return True
		#axarr[1].text(0.0, 0.9, 'sd',
		#			transform=axarr[1].transAxes, va='top')
		#plt.draw()
	else:
		return True


f.canvas.mpl_connect("motion_notify_event", hover)
f.canvas.mpl_connect('pick_event', onpick)

plt.show()
