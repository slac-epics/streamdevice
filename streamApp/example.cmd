#!./streamApp

dbLoadDatabase "O.Common/streamApp.dbd"
streamApp_registerRecordDeviceDriver

epicsEnvSet "STREAM_PROTOCOL_PATH", ".:protocols:../protocols/"

#setup the busses
#drvAsynSerialPortConfigure "COM2", "/dev/ttyS1"
drvAsynIPPortConfigure "terminal", "localhost:40000"

#load the records
dbLoadRecords "example.db"
#dbLoadRecords "scalcout.db"

#var streamDebug 1
iocInit
