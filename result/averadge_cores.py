import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdate
import numpy as np
import glob
import os 
import sys

folder= 'average'
files=sorted(glob.glob('*.csv'))

column= sys.argv[1]
print (column)
filtert_lines=[]
for name in files:
	df = pd.read_csv(name) #loading the data
	df=df.groupby(['cores', 'TestSizePre']).mean()
	df = df.reset_index()
	if not os.path.exists(folder):
		os.makedirs(folder)

	path='./'+folder+'/'+name
	print (path)
	df.to_csv(path,index=False)

	filtert_lines.append(df.loc[df['TestSizePre'] == 40000])

i=0
print(files)
print(files)
names=["Coarse List","FineList","Lazy","Lazy_mem","LockFree","LockFree_impr","LockFree_impr_mem","Optimistic","Optimistic_mem"]


#####################################
for list_type in range(9):
	size=filtert_lines[list_type]['cores']
	if(size.size>0):
		y=filtert_lines[list_type][column]
		y=y/1000
		# plotting the points  
		plt.plot(size, y,marker='o',label = names[list_type]) 
	
######################################


# naming the x axis 
plt.xlabel('cores') 
# naming the y axis 
plt.ylabel('Time')  
  

# giving a title to my graph 
plt.title('Compare lists') 
  
# function to show the plot 
# show a legend on the plot 
plt.legend() 
plt.show() 