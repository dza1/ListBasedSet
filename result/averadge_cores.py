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


def lode_data(testsize):
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

		filtert_lines.append(df.loc[df['TestSizePre'] == testsize])
	return(filtert_lines)

i=0
print(files)
names=["Coarse List","FineList","Lazy","Lazy_mem","LockFree","LockFree_impr","LockFree_impr_mem","Optimistic","Optimistic_mem"]

def plot (filtert_lines,plot_name,column,xlabel,ylable):


	#####################################
	for list_ard in range(9):
		size=filtert_lines[list_ard]['cores']
		y=filtert_lines[list_ard][column]
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
	plt.xscale('log', basex=2)
	#plt.show()
	path_svg="./plots/"+plot_name+".svg"
	path_pdf="../Protokoll/plots_pdf/"+plot_name+".pdf"
	figure = plt.gcf()  # get current figure
	figure.set_size_inches(16, 9)
	

	
	plt.savefig(path_svg, format="svg", bbox_inches = 'tight', transparent=True)
	subprocess.run(["pwd"])
	subprocess.run(["inkscape", "--file="+path_svg, "--export-area-drawing", "--without-gui", "--export-pdf="+path_pdf],check=True)
	
	plt.close()


filtert_lines=lode_data(20000)

plot(filtert_lines,"mixed_time_cores_20000","time mixed","Cores","Zeit [s]")

filtert_lines=lode_data(40000)
plot(filtert_lines,"mixed_time_cores_40000","time mixed","Cores","Zeit [s]")

filtert_lines=lode_data(120000)
plot(filtert_lines,"mixed_time_cores_120000","time mixed","Cores","Zeit [s]")
