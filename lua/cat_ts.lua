function cat(filename, lines, one )
	local file_des = assert(io.open(filename))
	if not lines then
		local data = file_des:read("a")
		print(data)
		file_des:close()
		return data
	else
		print(lines)
		local content = {}
		for i=1,lines do
			line = assert(file_des:read("L"))
			print("debug "..line)
			content[#content + 1]=line
		end
		file_des:close()
		if one==true then
			return content[lines]
		else
			return content
		end
	end
end

function command(cmd)
	local pFile = io.popen(cmd)
	local pData = pFile:read("a")
	pFile:close()
	return pData
end

function ls(dir)
	local f = io.popen(dir)
	local data = f:read("a")
	print(data)
end

first=cat("test.txt")
print("first \n"..first)
second=cat("test.txt", 2)
for k,v in pairs(second) do
	print(v)
end

third=cat("test.txt", 2, true)
print("third: "..third)

print("popen: "..command("ls -a"))

s = "dev-abcd"

if string.find(s, "dev%-*") then
	print("dev- is found")
else
	print("dev- is not found")
end
xC_t_file="xCloud_time"
os.remove("test1.txt")
xCloud_str=cat("xCloud_time")
print(xCloud_str)
print(string.match(xCloud_str, "%d+"))
os.execute("cat "..xC_t_file)

ls("/home/marshall/study/lua")
