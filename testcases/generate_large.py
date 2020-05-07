import random

f = open("large.txt","w")

for i in range(1000): # Threads
    for i in range(20): # Actions
        f.write(str(random.randrange(1, 2147483646)) + " ")
    
    f.write("\n")