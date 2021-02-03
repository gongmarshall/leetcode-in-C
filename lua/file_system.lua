local lfs=require "lfs"
local currentPath = lfs.currentdir()
print(currentPath)
for entry in lfs.dir(currentPath) do
	if entry~='.' and entry~='..' then
		print(entry)
	else
		print(". ..")
	end
end
