#! @TCL_SHELL@
#
#  $Id$
#

#load the avr-simulator package
proc locateInDirs {name dirs} {

  foreach dir ${dirs}  {
     set toTest ${dir}/${name}
     if { [file exists ${toTest}] } {
       return ${toTest}
     }
  }
  error "unable to locate ${name}"
}

if { $tcl_platform(platform) == "windows" } {
  set pathSeparator ";"
} else {
  set pathSeparator ":"
}
#set libraryFile [locateInDirs libsimulavr@AVR_TCL_MODULE_SUFFIX@ [list [file normalize ../../src/.libs] /lib /usr/lib]]
set libraryFile "${env(LIBSIMULAVR)}"
set libraryPath [file dirname ${libraryFile}]
set env(PATH) "${env(PATH)}${pathSeparator}${libraryPath}"
load ${libraryFile}

#create new device
set dev1 [AvrFactory_makeDevice [AvrFactory_instance] "atmega16"]

#set the clock cycle time [ns] to 124 = 4MHz clock
AvrDevice_SetClockFreq ${dev1} 125

# Exit magic register
set exitInstance [new_RWExit ${dev1}]
AvrDevice_ReplaceIoRegister ${dev1} 0x4f $exitInstance

# Write magic register (only stdout supported)
set writeToPipeInstance [new_RWWriteToFile ${dev1} "FWRITE" "-"]
AvrDevice_ReplaceIoRegister ${dev1} 0x52 ${writeToPipeInstance}

#load elf file to the device
AvrDevice_Load ${dev1} "main.elf"

#last exit instance
AvrDevice_RegisterTerminationSymbol ${dev1} "exit"

#systemclock must know that this device will be stepped from application
set sc [GetSystemClock]
$sc Add ${dev1}

#lets the simulation run until it terminates itself
$sc Endless

exit 0
