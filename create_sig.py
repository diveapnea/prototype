import numpy as np
import csv

size = 2000

x1 = np.linspace(0, size, size)
y1 = np.sin(x1)+5 + np.random.random([size])
y1_str = ["%.2fV" % x for x in y1]

x2 = np.linspace(10, size+10, size)
y2 = np.sin(x2)+ 9 + np.random.random([size])
y2_str = ["%.2fA" % x for x in y2]


y_final = [list(a) for a in zip(y1_str, y2_str)]

with open("input.txt", "w") as f:
    writer = csv.writer(f)
    writer.writerows(y_final)