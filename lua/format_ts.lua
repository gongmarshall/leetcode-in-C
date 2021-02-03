local var="abc"
domain="A"
path="b"
serial_number="c"
model_name="d"
XCLOUD_TIME_FILE="E"
cmd = string.format([[curl -X POST \"http://%s/%s\" -H \"content-type: application/json\"
-d \"{\"hardwareId\":\"%s\",\"model\":\"%s\" }\" --connect-timeout 3 -sS -k 
--fail -o \"%s\"]], domain, path, serial_number, model_name, XCLOUD_TIME_FILE)
print(cmd)
