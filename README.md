Welcome to our fork of the erlhdf5 binding, to manage the HDF5 file format from the Erlang/OTP programming language.


# User Notes

This is a fork of the [original erlhdf5 binding](https://github.com/RomanShestakov/erlhdf5), which was mainly a proof of concept. This fork goes a little further.

This binding allows to use [HDF5](https://en.wikipedia.org/wiki/Hierarchical_Data_Format#HDF5) from [Erlang](http://erlang.org), i.e. to read and write files in the HDF5 format.

While this fork is more complete and more correct (ex: HDF5 writing has been fixed), this binding is still very far from capturing all the numerous APIs provided by HDF5.


Compared to the original work, apart the low-level code enhancements, comments additions, typing improvements and bug fixing:
* datatype of stored elements can be now (native) integer or (native) double i.e. floating-point values; was: only native integers
* a basic hyperslab support has been added, so that only part of a in-file dataset can be updated (from in-memory data); previously: datasets had to be written in full (i.e. no dataspace size was specified, hence as many bytes as needed were read from RAM to fill the targeted dataset)



## Known binding limitations

This binding has following known limitations:
* larger datasets may incur performance penalties, since much data transformation is involved between C arrays and their Erlang counterparts
* many HDF5 datatypes and APIs not integrated


## Known limitations of this fork

This fork, compared to the original erlhdf5, has following known limitations:
* the Travis continuous integration and other fancy features like the automatic download and install of HDF5 have probably been broken (as we do not use them)



## About data arrays

Following is an integer array with 3 rows and 4 columns.

In Erlang, we make it correspond to a list of 3 tuples of 4 integer elements each:

```
[  {0, 1, 2, 3},
   {4, 5, 6, 7},
   {8, 9, 10, 11}
]
```


In C:

```
int a[3][4] = {
 {0, 1, 2, 3} ,   /*  initializers for row indexed by 0 */
 {4, 5, 6, 7} ,   /*  initializers for row indexed by 1 */
 {8, 9, 10, 11}   /*  initializers for row indexed by 2 */
};
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
