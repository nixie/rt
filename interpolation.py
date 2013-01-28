#!/usr/bin/env python
import sys
import glob
import numpy as np
from scipy import interpolate
import matplotlib.pyplot as plt
import os.path

fps=40;
debug=False;

if len(sys.argv) != 3:
    print('Not enough parameters!')
    sys.exit(1)

for i in range(1,3):
    if sys.argv[i][-1] != '/':
        sys.argv[i] += '/'

# open input files (keyframes)
cnt=0
fds=[]
while os.path.exists(sys.argv[1] + 'keyframe_' + str(cnt) + '.txt'):
    fds.append(open(sys.argv[1] + 'keyframe_' + str(cnt) + '.txt'))
    cnt += 1

# read input files - create vectors of values, we want to interpolate
Ys=[]
Yb=[]
stop=False
while not stop:
    arr=[]
    sflag=True
    for fd in fds:
        line=fd.readline()
        if len(line) == 0:
            stop=True
            break
        elif line.startswith('txt '):
            arr.append(line.strip())
            sflag=False
        else:
            arr.append(float(line.strip()))

    if not stop:
        if sflag:
            Ys.append(arr)
        else:
            Yb.append(arr)

# new x axis
xnew = np.arange(0,len(fds)-1,1/fps)

print('Number of frames: %d (%d keyframes, %d FPS)' % (len(xnew), cnt,fps ))
# open output file for each interpolated config file
newfds=[]
for (i,x) in enumerate(xnew):
    newfds.append(open(sys.argv[2] + 'config_' + str(i).rjust(5,'0') + '.txt', 'w'))

opts={}
opts[9]='Camera pos, X axis';
opts[10]='Camera pos, Y axis';
opts[11]='Camera pos, Z axis';
opts[12]='Camera yaw';
opts[13]='Camera pitch';
opts[14]='Camera roll';

# interpolate vectors in Ys
X=[x for x in range(0,len(fds))]
for (i,Y) in enumerate(Ys):
    tck = interpolate.splrep(X,Y,k=2,s=0)
    ynew = interpolate.splev(xnew,tck,der=0)

    if debug and i in (9,10,11, 12,13,14):
        # plot interpolated position using quadratic and cubic spline
        tck3 = interpolate.splrep(X,Y,k=3,s=0)
        ynew3 = interpolate.splev(xnew,tck3,der=0)
        plt.figure()
        plt.plot(X,Y,'x',xnew,ynew, xnew, ynew3, '--')
        plt.legend(['Keyframe values','Quadratic spline', 'Cubic spline'])
        plt.title('Spline interpolation of ' + opts[i])
        plt.grid()
        plt.show()

    for (i,y) in enumerate(ynew):
        newfds[i].write(str(y) + '\n')

# copy without interpolation boolean values from Yb
for (i,y) in enumerate(Yb):
    for (j,x) in enumerate(xnew):
        keyframe = int(x)
        newfds[j].write(y[keyframe] + '\n')

# close all files
for fd in fds:
    fd.close()
for fd in newfds:
    fd.close()
