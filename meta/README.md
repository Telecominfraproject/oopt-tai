Metadata for TAI
================

Metadata for TAI is a set of auto generated (based on TAI headers) data
and functions which allow TAI attributes serialization and more.

Metadata is generated as ANSI C source and header.

Parser also forces headers to be well formated when adding new code.

As same as TAI itself, TAI metadata is based upon and quite common to the
SAI metadata.


## How to build

### native build

#### prerequisite

- gcc
- python3
- libclang6.0

```sh
$ make
```

### Docker build

#### prerequisite

- docker

```sh
$ make docker-image
$ make docker
```
