tai shell
=
[1] Build
-
    cd ./tools/taish
    make all
    
[2] Usage
-
[NAME]

    taish - Provide the simple shell to control the optical module.
    
[SYNOPSIS]

    taish [-i IP_ADDRESS] [-p PORT]
    
[DESCRIPTION]

    The taish applcation provides the brief shell to control the optical modules via TAI. The taish application starts as
    the server process and wait the connection from the client on a given IP address and a given TCP port.
    
    The client can connect to the taish applciation via the given IP address and the given TCP port. The taish launches
    the brief shell and provides the commands to control the optical modules via TAI. The client can execute the commands
    provided by the taish application. As the client application, telnet command can be used.
    
    The options for the taish application are as follows:
    
    -i : Specify the IP address which is used by the taish application (0.0.0.0 as default)
    
    -p : Specify the TCP port number which is used by the taish applciation (4501 as default)
    
    
    The commands provided by the taish application are as follows:
    
    load <path_to_tai_library>: The taish appliation loads the TAI library corresponding to the optical modules.
    
    init: The taish applcation initiate the initialization of the optical modules via TAI library. The TAI library msut
          be loaded in advance.
    
    logset [debug|info|notice|warn|error|critical]: Set the logging level.
    
    set_netif_attr <module_id> <attribute_id> <attribute_value> : Set the network interface attribute.
        <module_id> : Numnber of the target module
        <attribute_id> : Attribute name
            tx-enable, tx-grid, tx-channel, output-power, tx-laser-freq, modulation, differential-encoding
        <attribute_value> : Value for a given attribute
            tx-enable : true or false
            tx-grid : 100, 50, 33, 25, 12.5 or 6.25
            tx-channel : integer
            output-power : float
            tx-laser-freq : integer
            modulation : bpsk, dp-bpsk, qpsk, dp-qpsk, 8qam, dp-8qam, 16qam, dp-16qam, 32qam, dp-32qam, 64qam or dp-64qam
            differential-encoding : true or false
            
    quit | exit : disconnection from the taish application
    
    help : Show help
    
    
    NOTE: 
    Normally, each chipset vendor provides their proprietary shell/tool to debug the chipset. The purpose of the taish
    application is to provide the vendor agnostic shell/tool to debug the optical modules via TAI.
    
