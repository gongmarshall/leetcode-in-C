local lfs = require "lfs"

local file_t = lfs.attributes("/bin/ip")
if file_t == nil then
	print("not found")
else
	print("is in /bin/ip")
end

local file_t1 = lfs.attributes("/home/marshall")
print(file_t1)

if (lfs.attributes("/local/ip")) then
	ip_path = "/local/ip"
elseif lfs.attributes("/bin/ip") then
	ip_path = "/bin/ip"
else 
	ip_path = "ip"
end
print(ip_path)
