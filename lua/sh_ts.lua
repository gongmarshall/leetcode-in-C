require('sh')

local wd = tostring(pwd()) -- calls `pwd` and returns its output as a string

local files = tostring(ls('/tmp')) -- calls `ls /tmp`
for f in string.gmatch(files, "[^\n]+") do
	print(f)
end
