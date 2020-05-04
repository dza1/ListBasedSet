import random

f = open("large.txt","w")

for i in range(1000): # Threads
    for i in range(2000): # Actions
        f.write(str(random.randrange(1, 1000)) + " ")
    
    f.write("\n")