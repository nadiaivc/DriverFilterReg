I have an article about it on Habr https://habr.com/ru/post/485606/

Based on https://github.com/microsoft/Windows-driver-samples/tree/master/general/registry/regfltr
      
#### The configuration will be stored in the XML format, Config file set in registry.(change "//" to "/" and del Space):
> < exe><name_program(name of the program for which we are setting permissions)*> <Lev* (access level)> *1* (value) <//Lev> ... (other programs) 
... *<//exe> < key> <name_key* (name of the key (section) , for which we set the rights)> *< Lev> <4> <//Lev> ........ <//key>*
### parsing configuration is pretty bad - better change
#### Installation and removal of the notifier (PsSetLoadImageNotifyRoutine) by the “+” and “-” command received through the IOCTL request mechanism from the developed user level application. 
#### For Stop driver - print 1 any symbol
The installed notifier logs in the file information about the event that occurred, indicating the time, process name and PID, module name.
### Some navigation:
#### exe - regctrl.c is application that sends IOCL requests to the driver
#### sys - driver.c, where are the functions of loading the driver, unloading and receiving IOCL requests from our application; regfltr.c, where is the function of receiving IRP packets (Callback)
