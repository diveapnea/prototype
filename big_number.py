import matplotlib
import matplotlib.pyplot as plt
import numpy as np

# Data for plotting
x = 10
y1 = x**(x**x)
y2 = x**x
style = '.-'

fig, ax = plt.subplots()
ax.plot(x, y1,style, x, y2,style)

ax.set(xlabel='x', ylabel='big number',
       title='Let it grow')
ax.grid()

#fig.savefig("test.png")
plt.show()