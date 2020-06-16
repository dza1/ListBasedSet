import random

FILE_AMOUNT = 1
RAW_BASE = 50 #amount of lines for the first file
COOLUMS_BASE = 50 #amount of coolums for the first line 


def create_testfiles():
    sw_rounds =cols*rows
    arr = [[0 for i in range(cols)] for j in range(rows)] 
    #Fill array
    for i in range (rows):
        for j in range (cols):
            arr[i][j]=j+(cols*i)+1

    #mix the array
    for i in range(sw_rounds):
        rn_row_1=random.randint(0, rows-1)
        rn_col_1=random.randint(0, cols-1)
        rn_row_2=random.randint(0, rows-1)
        rn_col_2=random.randint(0, cols-1)
        tmp =arr[rn_row_1][rn_col_1]
        arr[rn_row_1][rn_col_1 ] = arr[rn_row_2][rn_col_2]
        arr[rn_row_2][rn_col_2]=tmp


  ###### write the first part of values to the pre csv
    for i in range(rows): # Threads
        for j in range((int)(cols/2)): # Actions
            pre.write(str(arr[i][j]))
            if(j<((int)(cols/2)-1)):
                pre.write(",")
        pre.write("\n")


    ###### makes the first part of the array negative
    for i in range(rows): # Threads
        for j in range((int)(cols/2)): # Actions
            arr[i][j]=-arr[i][j]
           

    #mix the array
    for i in range(sw_rounds):
        rn_row_1=random.randint(0, rows-1)
        rn_col_1=random.randint(0, cols-1)
        rn_row_2=random.randint(0, rows-1)
        rn_col_2=random.randint(0, cols-1)
        tmp =arr[rn_row_1][rn_col_1]
        arr[rn_row_1][rn_col_1 ] = arr[rn_row_2][rn_col_2]
        arr[rn_row_2][rn_col_2]=tmp

    # write values to the main csv
    for i in range(rows): # Threads
        for j in range((int)(cols)): # Actions
            main.write(str(arr[i][j]))
            if(j<(cols-1)):
                main.write(",")
        main.write("\n")

#main function
print ("start")

for i in range(FILE_AMOUNT):
    pre_file="pre{0}.csv".format(i)
    main_file="main{0}.csv".format(i)
    pre = open(pre_file,"w")
    main = open(main_file,"w")
    rows, cols = (RAW_BASE*(i+1), COOLUMS_BASE*(i+1)) 
    create_testfiles()
    
print("finish")