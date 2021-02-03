file=io.open("syn_time_update","a")
file:close()

--check a file exist
file,message=io.open("test.txt")


print(file)
print(message)
