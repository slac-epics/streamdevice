BIN="<library directory>"
DBD="<dbd directory>"
HOME="<ioc home direcory>"

STREAM_PROTOCOL_PATH=".:protocols:../protocols/"

cd BIN
ld < iocCore
ld < streamApp.munch
dbLoadDatabase("streamApp.dbd",DBD)

drvAsynIPPortConfigure "terminal", "xxx.xxx.xxx.xxx:40000"

#load the records
cd HOME
dbLoadRecords("example.db")

#streamDebug=1
iocInit
