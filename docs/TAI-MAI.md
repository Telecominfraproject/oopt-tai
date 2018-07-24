TAI-MAI : An Enhanced Generic Interface 
=======================================
TAI (Transponder Abstraction Interface)
MAI (Module Abstraction Interface)


TAI (Transponder Abstraction Interface)
=======================================

The Transponder Abstraction Interface, or TAI, defines the API to provide a
Platform Independent way to control transponders from various vendors. TAI provides 
implementations which are uniform manner to configure underneath module hardware via MAI.
It is based upon Switch Abstraction Interface, or SAI.

MAI (Module Abstraction Interface)
=======================================

The Module Abstraction Interface, or MAI, defines a similiar API as TAI to provide a
Module Independent way to control modules from various transciever vendors. MAI provides 
implementations which are uniform manner to configure underneath hardware module.

Based on the implmentation of the platform, the MAI is an optional layer. Example of 
platforms where there is only one type of module supported on the platform.
In case of platform like Cassini where multiple moodule type are supported, MAI 
provides a uniform access interface.

Components
-----------

TAI is an interface specification implemented as a collection of C-language 
header files. TAI adopts the use of many terms from SAI, including "Adapter" and 
"Adapter Host".

An Adapter Host is platform independent software which uses the TAI interface
to provide optical transponder functionality for north-bound interface of the system. 
The Adapter Host loads the platform dependent software, the Platform Adapter.

Platform Adapter is similar to a user mode driver. It translates from the platform
independent interface to a hardware specific implementation via TAI. This API
are implemented as a library, and will typically be provided by the Platform
Vendor. This library is called libPlatformAdaptor.so.

Module Adapter provides drivers to specific module. Their implementation are
hardware specific drivers, and they provide a uniform interface via
Module Abstract Interface (MAI) for Platform Adapter. They are typically provided
either by platform vendor or by module vendor working closely with platform
vendor. 
Examples of modules are Analog Coherent Optics (ACO) module and 
Digital Coherent Optics (DCO) modules. The modules support different transceivers
and those drivers are all part of Module Adapter.
The library are named based on the module vendors. 
Example are libModuleAcacia.so, libModuleNEL.so etc.,.


```
                    ┌────────────────────────────┐                   
                    │                            │                   
                    │        Adapter Host        │                   
                    │                            │                   
                    │                            │                   
                    └────────────────────────────┘                   
                                                                     
─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─TAI Interface─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
                                                                     
                    ┌────────────────────────────┐                   
                    │                            │                   
                    │      Platform Adapter      │                   
                    │                            │                   
                    │                            │                   
                    └────────────────────────────┘                   
                                                                     
                                                                     
 ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ MAI Interface ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ 
                                                                     
┌────────────────────────────┐         ┌────────────────────────────┐		
│                            │         │                            │
│      Module A Adapter      │         │      Module B Adapter      │
│                            │         │                            │
│                            │         │                            │
└────────────────────────────┘         └────────────────────────────┘
                                                                     
─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ Hardware Interface─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
                                                                     
 ┌────────────────────────────┐         ┌───────────────────────────┐
 │                            │         │                           │
 │        HW Module A         │         │        HW Module B        │
 │                            │         │                           │
 │                            │         │                           │
 └────────────────────────────┘         └───────────────────────────┘
```

The TAI Interface
-----------------

The Adapter Host begins using TAI by invoking the `tai_api_initialize()` 
function. This function results in Platform Adapter (PA) to initialize various data structures 
for subsequent use. PA will find the location of the modules present via different techinques
based on the platform like sysfs, ONLP etc.,.
PA will detect the corresponding Module Adapter (MA) driver library and loads them via dlopen API.
It stores the handle for each of the module library, to invoke the routine provided by the library.
PA will now invoke the MA `mai_api_initialize()`

Example: 
Location 1 has Acacia based Module, it will load libModuleAcacia.so. Saves as handle1.
Location 2 has NEL based Module, it will load , libModuleNEL.so. Saves as handle2.

The main functionality provided by PA are
a)Detection of Modules present in the platform, identify the vendor type and  its location in the platform.
b)Load the driver corresponding to the  Module Adapter DLL
c)Translate the Object ID provided by the Module Adapter to a unqiue Object ID for the platform,
d)Translate the location ID determined by PA to logical location ID for Host Adapter.

There are two parameters passed to the `tai_api_initialize()` function: `flags`, which 
is currently unused and must be set to zero, and  a pointer to `tai_service_method_table_t`, called
`services`. This is a pointer to a structure of function pointers. This structure 
provides the PA with function entry points (services) in the adapter host. 
There is currently only one member in the structure, 
`module_presence`, which the PA calls whenever there is a change in the 
presence or absence of modules in the system. 

It can be extended for additional function like platform method read, write, status, reset and alarms.

The PA can call the `module_presence()` function for each module in the 
system whenever module are physically inserted or removed.

The `module_presence()` function may be called from a different thread than the 
main adapter host thread. And since TAI functions are not thread-safe, the 
adapter host is required make sure that it does not re-enter the TAI interface 
from the `module_presence()` function. This can be done by, for example, 
enqueueing the notifications for later processing on the main adapter 
host thread.

After receiving the `tai_api_initialize()`  PA will in turn call Module Adapter's
`mai_api_initialize()` function.
There are two parameters passed to the `mai_api_initialize()` function: `flags`, which 
is currently unused and must be set to zero, and  a  null pointer for `mai_service_method_table_t`. called

The `tai_api_uninitialize()` function is the inverse function to 
`tai_api_initialize()` and is called when the adapter host has completed all TAI 
interface processing. For example, just before exiting. This function will undo 
what was done in `tai_api_initialize()`, freeing memory, closing files, stopping 
threads, etc, in order to gracefully wrap up execution of the adapter.

The above routine will trigger `mai_api_uninitialize()` function to the MAI. 
The implemenation can be vendor dependant. This routine can bring the module into a low power mode.

After `tai_api_initialize()` returns successfully, the adapter host is free to 
call the other TAI APIs. It does this by first obtaining a list of function 
entry points for each API. There are currently three APIs:

* Module
* Network Interface
* Host Interface

For each of these APIs, a list of function entry pointers is obtained by calling 
the `tai_api_query()` function. This function takes two parameters, 
`tai_api_id`, which identifies which of the three API's function pointers are 
being requested, and `api_method_table`, the address of a pointer which the 
platform adapter will set to point to the method table for that API.

A method table contains function entry pointers to the functions which 
implement the API. Currently, all three APIs have very similar method tables 
which contain six functions:

* Create an object
* Destroy an object
* Set the value of an attribute of an object
* Set the values for a list of attributes of an object
* Get the value of an attribute of an object
* Get the values for a list of attributes of an object

### TAI Objects

The PA provides unique Object ID for each of the `tai_api_id` to the host.
PA uses the Object ID of the MAI and converts them to a unique value, 
using location information and returns to the Host Adapter.

PA simple acts as a pass through for all the objects and attribute API by calling
the MA. MA maintains all the objects and its attributes.
When host requests for a attribute value for an object, PA will map the object
to the corresponding MA. Call the similiar API of MA to obtain the result.

Each API allows objects to be created through the `create_xxx()` method table 
function. This function creates an instance of the object and initializes 
its attributes. When an object is no longer needed the adapter host can call
the `remove_xxx()` method table function, which will remove that object and its 
attributes. The `set_xxx_attribute()` function can be called once an object has 
been created to modify the value of one of the object's attributes. The 
`get_xxx_attribute()` function is used to retrieve the value of an object's 
attribute. The `set_xxx_attributes()` and `get_xxx_attributes()` functions
(notice the 's' at the end of the function names) are provided for convenience 
to set and get a list of attributes in a single function call.

### MAI Objects

The Module Adapter maintains the Object ID for each of the `tai_api_id` to the PA.
Each MA creates and maintains its Objects. It also maintains the Object attributes.
The object scope is local and will be mapped to platform space by PA.

#### Module Objects

The module API allows module objects to be created. A module object represents 
an optical module line card which are inserted into the platform. 
Calling the `create_module()` function causes the optical 
module to be initialized, which typically involves resetting the module and 
bringing it into a default state. This function must be called prior to creating 
host interface or network interface objects on a module. The `create_module()` 
function takes four parameters:

* A pointer to a tai_object_id_t. The MA will return PA the allocated module object
ID. PA will convert this to unquie ID for the adapter host.
This ID will be used by host adapter in subsequent TAI function calls.
* attr_count and attr_list. A list of attributes which will be set upon the 
module object when it is created. This can be used to override the default 
attribute values for a module.
* notification functions. A pointer to a list of function pointers in the 
adapter host which the adapter can call to notify the adapter host of changes in 
status of the module.

The attr_list supplied on the `create_module()` function call must contain a 
`TAI_MODULE_ATTR_LOCATION` attribute. This attribute defines which module is 
being created. This is typically set to the value of the module_location 
parameter from the `module_presence()` function.

An adapter host will typically call `create_module()` once for each module 
present in the system, as indicated when the adapter calls the 
`module_presence()` function.

Once the adapter host no longer needs to access the module it should call the 
`remove_module()` function. This can occur as the result of the 
`module_presence()` function being called indicating the module was removed, or 
when the adapter host is exiting, or whenever the adapter host feel like it. 
Removing the module simply makes it unaccessible to the adapter host. The actual 
state of the module is undefined. 

The PA will call MA `remove_module` function. MA may put the module in reset,
or may just leave the module in the same state it was prior to the 
`remove_module()` call. After calling `remove_module()` the object ID for the 
module which was returned in the `create_module()` function is invalid and 
should not be used in any subsequent TAI API calls.

Once the `create_module()` function returns with an object ID for a module, the 
adapter host can use this object ID to get and set attributes of the module with 
the `get_module_attribute()`, `get_module_attributes()`, 
`set_module_attribute()`, and `set_module_attributes()` function calls. Most of 
the module attributes are read-only and cannot be modified with the 
`set_module_attribute()` and `set_module_attributes()` functions.

All the above calls will be passed from PA to MA. MA maintains the attributes and 
the result of the call will be transfered from PA to host adapter.

Host interface objects and network interfaces objects are associated with the 
module on which they exist. This association is made when a host interface or 
network interface object is created, since the object ID of the associated 
module is a parameter of those `create_host_interface()` and 
`create_network_interface()` function calls. Because of this association, host 
interface and network interface objects should be removed by calling 
`remove_host_interface()` or `remove_network_interface()` prior to removing the 
module with which they are associated.

One of the attributes of a module object is the number of host interface and 
another attribute is the number of network interfaces. These values can be used 
when determining how many host interface objects and network interface objects 
exist on the module, and the index values used when calling 
`create_host_interface()` and `create_network_interface()`.

#### Host Interface Objects

The host interface API allows host interface objects to be created. A host 
interface object represents an interface between an optical module and the 
host system, sometimes called client interfaces. Creating a host interface 
object allows the attributes of that interface to be set and retrieved. The 
`create_host_interface()` function takes four parameters:

* A pointer to a tai_object_id_t. The MA will return PA the allocated module object
ID. PA will convert this to unique ID for the adapter host.
This ID will be used by host adapter in subsequent TAI function calls.
* The module ID. Identifies the module ID upon which the host interface exists.  
This object ID was returned when the module was created.
PA will use the MA returned object id to MA.
* attr_count and attr_list. A list of attributes which will be set upon the 
host interface object when it is created. This can be used to override the 
default attribute values for a host interface.

The attr_list must contain an `TAI_HOST_INTERFACE_ATTR_INDEX` attribute. This 
attribute defines which host interface is being created. This is a zero-based 
index of the host interfaces on a module.

An adapter host will typically call `create_host_interface()` once for each 
host interface present in the module, as indicated by the number of host 
interfaces, which is a module object attribute.

Once the adapter host no longer needs to access the host interface it should 
call the `remove_host_interface()` function. This can occur as the result of the 
`module_presence()` function being called indicating the module was removed, or 
when the adapter host is exiting, or whenever the adapter host feel like it. 
Removing the host interface simply makes it unaccessible to the adapter host. 
The actual state of the host interface is undefined. The adapter may put the  
host interface in reset, or may just leave the host interface in the same state 
it was prior to the `remove_host_interface()` call. After calling 
`remove_host_interface()` the object ID for the host interface which was 
returned in the `create_host_interface()` function is invalid and should not be 
used in any subsequent TAI API calls.

Once the `create_host_interface()` function returns with an object ID for a 
host interface, the adapter host can use this object ID to get and set 
attributes of the module with the `get_host_interface_attribute()`, 
`get_host_interface_attributes()`, `set_host_interface_attribute()`, and 
`set_host_interface_attributes()` function calls.

#### Network Interface Objects

The network interface API allows network interface objects to be created. A 
network interface object represents an interface between the optical module and 
the optical line system. Creating a network interface object allows the 
attributes of that interface to be set and retrieved. The
`create_network_interface()` function takes four parameters:

* A pointer to a tai_object_id_t. The adapter will return a pointer to the newly 
allocated network interface ID object. The adapter host should treat this object 
as opaque. It will be used in subsequent TAI function calls. 
* The module ID. Identifies the module ID upon which the network interface 
exists. This object ID was returned when the module was created.
* attr_count and attr_list. A list of attributes which will be set upon the 
network interface object when it is created. This can be used to override the 
default attribute values for a network interface.

The attr_list must contain an `TAI_NETWORK_INTERFACE_ATTR_INDEX` attribute. This 
attribute defines which network interface is being created. This is a zero-based 
index of the network interfaces on a module.

An adapter host will typically call `create_network_interface()` once for each 
network interface present in the module, as indicated by the number of network 
interfaces, which is a module object attribute.

Once the adapter host no longer needs to access the network interface it should 
call the `remove_network_interface()` function. This can occur as the result of
the `module_presence()` function being called indicating the module was removed, 
or when the adapter host is exiting, or whenever the adapter host feel like it. 
Removing the network interface simply makes it unaccessible to the adapter host. 
The actual state of the network interface is undefined. The adapter may put the  
network interface in reset, or may just leave the network interface in the same
state it was prior to the `remove_network_interface()` call. After calling 
`remove_network_interface()` the object ID for the network interface which was 
returned in the `create_network_interface()` function is invalid and should not
be used in any subsequent TAI API calls.

Once the `create_network_interface()` function returns with an object ID for a 
network interface, the adapter host can use this object ID to get and set 
attributes of the module with the `get_network_interface_attribute()`, 
`get_network_interface_attributes()`, `set_network_interface_attribute()`, and 
`set_network_interface_attributes()` function calls.

TAI Attributes
--------------

All Attributes values for the objects are maintained in MA. PA will act as a
pass through to set and get the attributes for the objects.

Each object has a list of attributes which are specific to that type of object. 
Each attribute is assigned an ID which is used to get or set the attribute's 
value. The TAI header files for each API list the attributes for each object, 
including descriptions of the format and access types. For example, the 
taimodule.h header file lists the module object attributes. The 
`TAI_MODULE_ATTR_MAX_LASER_FREQ` attribute, for example, has a short description 
(The TX/RX maximum laster frequency in Hz), the data type (tai_uint64_t), and 
the access rules (READ_ONLY). Unless otherwise stated, attributes can be set and
retrieved at any time after the object is created and prior to its destruction. 
Modifying an attribute will commonly cause a modification in the operational 
state of a module. Refer to the taimodule.h, taihostif.h and tainetworkif.h 
files for a list of the attributes of each object type.

TAI Return Codes
----------------

Every TAI API function returns one of the codes in the taistatus.h file. 
Successful execution of the function is indicated by the TAI_STATUS_SUCCESS 
code. A failure is indicated by any value other than TAI_STATUS_SUCCESS.

When processing a list of attributes, the return code can indicate not just 
that a failure occurred, but also which attribute in the list caused that 
failure. The return codes TAI_STATUS_INVALID_ATTRIBUTE_0, 
TAI_STATUS_INVALID_ATTR_VALUE_0, TAI_STATUS_ATTR_NOT_IMPLEMENTED_0,
TAI_STATUS_UNKNOWN_ATTRIBUTE_0, and TAI_STATUS_ATTR_NOT_SUPPORTED_0 are spaced 
out by 64K. Therefore, the least significant 16 bits of each of those error 
codes contains the index into the attribute list of the attribute which caused 
the error.
