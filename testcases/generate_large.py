import random

f = open("large.txt","w")

for i in range(10): # Threads
    for i in range(2000): # Actions
        f.write(str(random.randrange(1, 2147483646)) + " ")
    
    f.write("\n")