#!/usr/local/bin/lua

local date = os.time()
local day2year = 365.242
local sec2hour = 60*60
local sec2day = sec2hour*24
local sec2year = sec2day * day2year

--year
print(date // sec2year + 1970)

--hour
print(date % sec2day // sec2hour)

-- minutes
print(date % sec2hour//60)
--seconds
print(date % 60)

