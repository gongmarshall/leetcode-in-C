local _M = {
	BASEDIR = "/etc/net6conf",
	CONFIG	= "/bin/config",
	ABC = _M.CONFIG
}

_N = _M
function _M.test1()
	return "hello world"
end
_N.LAN = _M.CONFIG.." get lan"
return _N
