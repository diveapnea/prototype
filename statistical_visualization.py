import matplotlib
import matplotlib.pyplot as plt
import numpy as np

# Data for plotting
x = [0, 0.1, 1, 2, 3, 4, 5,10, 11, 12]
y = [0, 1, 2, 3, 4, 5]
style = '.-'

fig, ax = plt.subplots()
ax.boxplot([x,y])

ax.set(xlabel='data', ylabel='big number',
       title='Let it grow')
ax.grid()

#fig.savefig("test.png")
plt.show()