TAI Shell (taish)
---

taish is a tool/library which provides TAI debug shell/api function.
It comes with 3 components.

1. standalone taish server `taish-server`
2. command line interface `taish`
3. taish library `libtaigrpc.so`

taish uses [gRPC](https://grpc.io/) for the server/client communication.
`taish-server` is a simple standalone tool written in C++ which provides taish gRPC API.
As for TAI handling, it just creates module, netif and hostif with
mandatory attributes and do nothing else.

You can access the taish gRPC API using `taish` command. It is written in Python
and provides human friendly interactive shell interface.

By using `libtaigrpc.so`, you can also put taish server functionality into
your TAI application. Since `libtaigrpc.so` provides the exactly same gRPC API as
`taish-server`, you can use `taish` command.

## How to build `taish-server`

### native build

#### prerequisite

- make
- g++
- grpc

```
$ make
```


### docker build (recommended)

#### prerequisite

- make
- docker

```
$ cd $(TAI_TOP_DIR)
$ make docker-image # build container image for building TAI components
$ make bash # run bash in the container
root@ubuntu-bionic:/data# cd tools/taish/
root@ubuntu-bionic:/data/tools/taish# make
```

## Usage

### `taish-server`

```
$ ./taish-server
```

By default `taish-server` listens on `0.0.0.0:50051`. You can change it by using
`-i` and `-p` options.

```
$ ./taish-server -i 127.0.0.1 -p 10000
```

### `taish`

```
$ ./client/taish.py --addr <ip address of the server>
>
```

#### `list` command

```
> list
module: 1 0x1000000000000
 hostif: 0 0x2000000000000
 hostif: 1 0x2000000000001
 netif: 0 0x3000000000000
module: 2 0x1000000000001
 hostif: 0 0x2000000000100
 hostif: 1 0x2000000000101
 netif: 0 0x3000000000100
>
```

`list` command displays all module, netif and hostif which are created by TAI
application.

In the example output above, it shows that 2 modules whose [location](https://github.com/Telecominfraproject/oopt-tai/blob/master/inc/taimodule.h#L122) is
'1' and '2' respectively are created. The module '1' has 2 host interfaces whose [index](https://github.com/Telecominfraproject/oopt-tai/blob/master/inc/taihostif.h#L92) is 0 and 1,
and 1 network interface whose [index](https://github.com/Telecominfraproject/oopt-tai/blob/master/inc/tainetworkif.h#L155) is 0.
The output also shows object id of them.


#### `module` command

```
> module 1
module(1)>
```

`module` command selects which module to handle in successive commands.
You need to specify valid location which can be known by `list` command.

After executing the command, the prompt will change to show you which module
is selected.

#### `netif`, `hostif` command

```
> netif 0
no module selected.
> module 1
module(1)> netif 0
module(1)/netif(0)> hostif 0
module(1)/hostif(0)> hostif 1
module(1)/hostif(1)>
```

`netif` and `hostif` commands select which network interface or host interface
to handle in successive commands. You need to specify valid index which can be
known by `list` command.

Also, before using these commands, you need to specify `module` first.
In the example output above, the first command `netif 0` is failed since no
module was selected.

As same as `module` command, the prompt will change to show you which netif/hostif
is selected after executing the command.

#### `quit` command

```
module(1)/hostif(1)> q
module(1)> q
```

This command is used to unselect the current selected TAI objected.

#### `list-attr` command

```
module(1)> list-attr
name                            readonly    value
------------------------------  ----------  ------------------------------
location                        false       <string>
vendor-name                     true        <string>
vendor-part-number              true        <string>
vendor-serial-number            true        <string>
firmware-versions               true        <flaot list>
oper-status                     true        [unknown|initialize|ready|max]
temp                            true        <float>
power                           true        <float>
num-host-interfaces             true        <uint32>
num-network-interfaces          true        <uint32>
admin-status                    false       [unknown|down|up|max]
tributary-mapping               false       <oid map list>
module-shutdown-request-notify  false       <pointer>
module-state-change-notify      false       <pointer>
module(1)>
module(1)>
module(1)>
module(1)> netif 0
module(1)/netif(0)> list-attr
name                               readonly    value
---------------------------------  ----------  -----------------------------------------------------------------------------------------------------------------------------
index                              false       <uint32>
tx-align-status                    true        [loss|out|cmu-lock|ref-clock|timing]
rx-align-status                    true        [modem-sync|modem-lock|loss|out|timing]
tx-enable                          false       <bool>
tx-grid-spacing                    false       [unknown|100-ghz|50-ghz|33-ghz|25-ghz|12-5-ghz|6-25-ghz|max]
output-power                       false       <float>
current-output-power               true        <float>
tx-laser-freq                      false       <uint64>
tx-fine-tune-laser-freq            false       <uint64>
modulation-format                  false       [unknown|bpsk|dp-bpsk|qpsk|dp-qpsk|8-qam|dp-8-qam|16-qam|dp-16-qam|32-qam|dp-32-qam|64-qam|dp-64-qam|max]
current-ber                        true        <float>
current-ber-period                 true        <uint32>
differential-encoding              false       <bool>
oper-status                        false       [unknown|reset|initialize|low-power|high-power-up|tx-off|tx-turn-on|ready|tx-turn-off|high-power-down|fault|max]
min-laser-freq                     true        <uint64>
max-laser-freq                     true        <uint64>
laser-grid-support                 true        [unknown|100-ghz|50-ghz|33-ghz|25-ghz|12-5-ghz|6-25-ghz|max]
current-input-power                true        <float>
current-post-voa-total-power       true        <float>
current-provisioned-channel-power  true        <float>
pulse-shaping-tx                   false       <bool>
pulse-shaping-rx                   false       <bool>
pulse-shaping-tx-beta              false       <float>
pulse-shaping-rx-beta              false       <float>
voa-rx                             false       <float>
loopback-type                      false       [none|shallow|deep|max]
prbs-type                          false       [none|prbs7|prbs9|prbs11|prbs15|prbs20|prbs23|prbs31|max]
module(1)/netif(0)>
module(1)/netif(0)>
module(1)/netif(0)>
module(1)/netif(0)> q
module(1)> hostif 0
module(1)/hostif(0)> list-attr
name             readonly    value
---------------  ----------  -------------------------------------
index            false       <uint32>
lane-fault       true        [loss-of-lock|tx-fifio-err]
tx-align-status  true        [cdr-lock-fault|loss|out|deskew-lock]
fec-type         false       [none|rs|fc]
loopback-type    false       [none|shallow|deep|max]
module(1)/hostif(0)>

```

`list-attr` command displays all TAI attributes for selected TAI object.
If you execute this command when you are selecting a module, it will list all
[TAI module attributes](https://github.com/Telecominfraproject/oopt-tai/blob/master/inc/taimodule.h).
Same for netif and hostif.

The output also shows attribute type and readonly flag.
If the attribute type is enum, it shows valid enum values.

#### `get` command

```
module(1)> get location
1
module(1)> get oper-status
ready
module(1)>
```

`get` command gets TAI attribute. You need to specify valid attribute name
which can be known by `list-attr` command.

#### `set` command

```
module(1)/netif(0)> set output-power -1.0
module(1)/netif(0)> set modulation-format dp-16-qam
module(1)/netif(0)> set current-ber
attribute current-ber is read-only
```

`set` command sets TAI attribute. You need to specify valid attribute name and
value which can be known by `list-attr` command.
Obviously, you can't set a value to read-only attributes.
