import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdate
import numpy as np
import glob
import os 
import sys
import subprocess

folder= 'average'
files=sorted(glob.glob('*.csv'))

#column= sys.argv[1]
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

	filtert_lines.append(df.loc[df['cores'] == 64])

i=0
print(files)
names=["Coarse List","FineList","Lazy","Lazy_mem","LockFree","LockFree_impr","LockFree_impr_mem","Optimistic","Optimistic_mem"]

def plot (plot_name,column,xlabel,ylable,yscale):
	#####################################
	for list_ard in range(9):
		size=filtert_lines[list_ard]['TestSizePre']
		y=filtert_lines[list_ard][column]
		y=y/yscale
		# plotting the points  
		if(y.max()>0):
			plt.plot(size, y,marker='o',label = names[list_ard]) 
	######################################

	# naming the x axis 
	plt.xlabel(xlabel) 
	# naming the y axis 
	plt.ylabel(ylable)  
	# plt.title(plot_name) 
	plt.legend() 
	plt.grid()
	path_svg="./plots/"+plot_name+".svg"
	path_pdf="../Protokoll/plots_pdf/"+plot_name+".pdf"
	figure = plt.gcf()  # get current figure
	figure.set_size_inches(16, 9)


	plt.savefig(path_svg, format="svg", bbox_inches = 'tight', transparent=True)
	subprocess.run(["pwd"])
	subprocess.run(["inkscape", "--file="+path_svg, "--export-area-drawing", "--without-gui", "--export-pdf="+path_pdf],check=True)
	plt.close()

plot("write_time","time write","Listengröße","Zeit [s]",1000)
plot("mixed_time","time mixed","Listengröße","Zeit [s]",1000)
plot("check_time","time check","Listengröße","Zeit [s]",1000)
plot("mixed_goToStart","goToStart mixed","Listengröße","Anzahl",1)
plot("mixed_lostTime","lostTime mixed","Listengröße","Zeit [s]",1)