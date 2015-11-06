Welcome to our fork of the erlhdf5 binding, to manage the HDF5 file format from the Erlang/OTP programming language.


# User Notes

This is a fork of the [original erlhdf5 binding](https://github.com/RomanShestakov/erlhdf5), which was mainly a proof of concept. This fork goes a little further.

This binding allows to use [HDF5](https://en.wikipedia.org/wiki/Hierarchical_Data_Format#HDF5) from [Erlang](http://erlang.org), i.e. to read and write files in the HDF5 format.

While this fork is more complete and more correct (ex: HDF5 writing has been fixed), this binding is still very far from capturing all the numerous APIs that are provided by HDF5.


Compared to the original work, apart the low-level code enhancements, comments additions, typing improvements and bug fixing:
* datatype of stored elements can be now (native) integer or (native) double i.e. floating-point values; was: only native integers
* a basic hyperslab support has been added, so that only part of a in-file dataset can be updated (from in-memory data); previously: datasets had to be written only in full (i.e. no dataspace size was specified, hence as many bytes as needed were read from RAM to fill the targeted dataset, possibly with unexpected extra bytes taken to fill the target space)
* ```h5lt_read_dataset_double/2``` added


## Known binding limitations

This binding has following known limitations:
* no more than 2 dimensions allowed for dataspaces (rank=2)
* larger datasets may incur performance penalties, since much data transformation is involved between C arrays and their Erlang counterparts
* many HDF5 datatypes and APIs have not yet been integrated


## Known limitations of this fork

This fork, compared to the original erlhdf5, has following additional known limitations:
* the Travis continuous integration and other fancy features like the automatic download and install of HDF5 have probably been broken (as we do not use them)



## Installation Instructions


### Building your own HDF5 library

In some cases, a pre-installed HDF5 package will not work appropriately (ex: the binding will fail to compile as MPI includes are not found, or there will be API mismatches like: ```too many arguments to function 'H5Dcreate1'``` that would not all be solved by using ```-DH5Dcreate_vers=2```).

Then instead we can [fetch the sources](http://www.hdfgroup.org/ftp/HDF5/current/src/) of the latest version of HDF (ex: ```1.8.15-patch1```, at the time of this writing) and build them with:

```
 $ export HDF_VERSION=hdf5-1.8.15-patch1
 $ mkdir -p /home/foobar/Software/HDF/$HDF_VERSION-install
 $ cd /home/foobar/Software/HDF
 $ tar xvjf ~/$HDF_VERSION.tar.bz2
 $ cd $HDF_VERSION
 $ ./configure --prefix=/home/foobar/Software/HDF/$HDF_VERSION-install
 $ make && make check && make install
```


Then the root makefile of erlhdf5 can be updated regarding following variables:

```
HDF_VERSION := "1.8.15-patch1"
HDF_INSTALL := "/home/foobar/Software/HDF/hdf5-$(HDF_VERSION)-install"

ERLCFLAGS := "-g -Wall -fPIC -I$(HDF_INSTALL)/include -I$(ERL_LIB)/lib/erl_interface-3.7.20/include -I$(ERL_LIB)/erts-6.4/include -I/usr/lib/openmpi/include"
```


Having built once for all our own library, replace, still in ```erlhdf5/Makefile```, ```all:$(LIBRARY)``` by ```all:#$(LIBRARY)```.

Replace ```erlhdf5/rebar.config``` with following content (adapted w.r.t. paths):

```
{erl_opts, [debug_info, warnings_as_errors]}.
{lib_dirs,["deps"]}.
{sub_dirs, ["rel"]}.
{src_dirs, ["src", "test"]}.

{port_sources, ["c_src/*.c"]}.
{cover_enabled, true}.

{port_env, [
			{"CC", "h5cc"},
			{"PATH", "/home/foobar/Software/HDF/hdf5-1.8.15-patch1-install/bin:$PATH"},
			{"LDFLAGS", "-shlib -L/home/foobar/Software/HDF/hdf5-1.8.15-patch1-install/lib"},
			{"DRV_CFLAGS","-g -Wall -fPIC $ERL_CFLAGS"}
	   ]
}.
{port_specs, [{"priv/erlhdf5.so", ["c_src/*.c"]}]}.

```

Then everything should go smoothly:

```
 $ cd /home/foobar/Software/erlhdf5
 $ make clean all
./rebar clean
==> erlhdf5 (clean)
rm -rf test/*.beam
Compiling with rebar
CC=deps/hdf5/bin/h5cc CFLAGS="-g -Wall -fPIC -I"/home/foobar/Software/HDF/hdf5-"1.8.15"-install"/include -I""/home/foobar/Software/Erlang/Erlang-current-install"/lib/erlang/"/lib/erl_interface-3.7.20/include -I""/home/foobar/Software/Erlang/Erlang-current-install"/lib/erlang/"/erts-6.4/include -I/usr/lib/openmpi/include" ./rebar compile
==> erlhdf5 (compile)
make[1]: Entering directory `/home/foobar/Software/erlhdf5'
[...]
Compiling c_src/erlhdf5.c
```


Knowing that

```
$ export LD_LIBRARY_PATH=/home/foobar/Software/HDF/hdf5-1.8.15-patch1-install/lib:$LD_LIBRARY_PATH

is not necessary, we have now:
```

```
 $ cd erlhdf5
 $ erl -pz ./ebin/
Erlang/OTP 17 [erts-6.4] [source] [64-bit] [smp:8:8] [async-threads:10] [hipe] [kernel-poll:false]

Eshell V6.4  (abort with ^G)
1> erlhdf5:h5fopen( "dataset.nc", 'H5F_ACC_RDONLY').
{ok,16777216}
```

Everything is thus properly linked.




## About data arrays

Let's suppose we want to manipulate an integer array with 3 rows and 4 columns.

In Erlang, we make it correspond to a list of 3 tuples of 4 integer elements each:

```
[  { 0,  1,  2,  3 },
   { 4,  5,  6,  7 },
   { 8,  9, 10, 11 }
]
```


In C:

```
int a[3][4] = {
 { 0, 1, 2, 3   },   /*  initializers for row indexed by 0 */
 { 4, 5, 6, 7   },   /*  initializers for row indexed by 1 */
 { 8, 9, 10, 11 }    /*  initializers for row indexed by 2 */
} ;
```

In memory, this will be a continuous series of integers: `{0,1,2,3,4,5,6,7,8,9,10,11}`.



## About data conversions


HDF5 will perform some automatic conversions on the stored datatypes.

For example, if writing `Data = [ { 1.5, 2.5 }, { 3.5, 4.5 } ]` in a dataset using the `H5T_NATIVE_INT` type, then the HDF5 file will contain:

```
	 DATATYPE  H5T_STD_I32LE
	  DATASPACE  SIMPLE { ( 3, 4 ) / ( 3, 4 ) }
	  DATA {
	  (0,0): 1, 2, 3, 4,
	  (1,0): 0, 0, 0, 0,
	  (2,0): 0, 0, 0, 0
```

(note that here the writing is too short to fill the target dataspace - this should never be attempted with `h5dwrite/2`; a target dataspace and `h5dwrite/3` should be used for that)




# Developer Notes

All failure cases should deal with memory allocation, to avoid leaks.

A higher-level Erlang abstraction of HDF5 services has been built on top of this version of erlhdf5: see the `hdf5_support` module in [Ceylan-Myriad](https://github.com/Olivier-Boudeville/Ceylan-Myriad) (in `src/data-management`).


## Bugs

With the original binding, there was a bug in the writing of datasets.

Indeed, when writing `Data = [ { 1, 2, 3, 4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 } ]`, in the following dataspace:

```
{ ok, Space } = erlhdf5:h5screate_simple( 2, { 3, 4 } ),
```

In our version, we read:
```
data : [1,2,3,4,5,6,7,8,9,10,11,12]
```

and `h5dump` tells us:

```
DF5 "foobar.h5" {
GROUP "/" {
   DATASET "dset" {
	  DATATYPE  H5T_STD_I32LE
	  DATASPACE  SIMPLE { ( 3, 4 ) / ( 3, 4 ) }
	  DATA {
	  (0,0): 1, 2, 3, 4,
	  (1,0): 5, 6, 7, 8,
	  (2,0): 9, 10, 11, 12
	  }
   }
}
}
```

This looks correct.


In the original erlhdf5 version, we had:

```
data : [805614400,11166,805614432,11166,805614464,11166,37,0,1,2,3,4]
```

and `h5dump` was telling us:


```
HDF5 "foobar.h5" {
GROUP "/" {
   DATASET "dset" {
	  DATATYPE  H5T_STD_I32LE
	  DATASPACE  SIMPLE { ( 3, 4 ) / ( 3, 4 ) }
	  DATA {
	  (0,0): 805614400, 11166, 805614432, 11166,
	  (1,0): 805614464, 11166, 37, 0,
	  (2,0): 1, 2, 3, 4
	  }
   }
}
}
```

(showing traces of likely memory corruption)



## Troubleshooting

If, when expanding the coverage of the binding, you have a message like ```{"init terminating in do_boot",{nif_library_not_loaded,{module,erlhdf5},{line,286}}}```, then you may have forgotten to declare the corresponding function in ``static ErlNifFunc nif_funcs[]`` (in ``c_src/erlhdf5.c``).
