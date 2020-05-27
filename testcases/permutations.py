import random

FILE_AMOUNT = 5



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


    for i in range(rows): # Threads
        for j in range((int)(cols/2)): # Actions
            pre.write(str(arr[i][j]) + " ")
        pre.write("\n")




    ######Post file, negate the 2nd part of the array afterwards   
    for i in range(rows): # Threads
        for j in range((int)(cols/2),cols): # Actions
            # post.write(str(arr[i][j]) + " ")
            arr[i][j]=-arr[i][j]
        # post.write("\n")


    #mix the array
    for i in range(sw_rounds):
        rn_row_1=random.randint(0, rows-1)
        rn_col_1=random.randint(0, cols-1)
        rn_row_2=random.randint(0, rows-1)
        rn_col_2=random.randint(0, cols-1)
        tmp =arr[rn_row_1][rn_col_1]
        arr[rn_row_1][rn_col_1 ] = arr[rn_row_2][rn_col_2]
        arr[rn_row_2][rn_col_2]=tmp

    for i in range(rows): # Threads
        for j in range((int)(cols)): # Actions
            main.write(str(-arr[i][j]) + " ")
        main.write("\n")

for i in range(FILE_AMOUNT):
    pre_file="pre{0}.txt".format(i)
    main_file="main{0}.txt".format(i)
    pre = open(pre_file,"w")
    main = open(main_file,"w")
    # post = open("post.txt","w")

    rows, cols = (100, 100*(i+1)) 
    create_testfiles()