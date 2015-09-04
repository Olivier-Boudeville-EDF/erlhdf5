
/* Copyright (C) 2012 Roman Shestakov */

/* This file is part of erlhdf5 */

/* erlhdf5 is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as */
/* published by the Free Software Foundation, either version 3 of */
/* the License, or (at your option) any later version. */

/* erlhdf5 is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public */
/* License along with Erlsom.  If not, see */
/* <http://www.gnu.org/licenses/>. */

/* Author contact: romanshestakov@yahoo.co.uk */

/*
 Forked on Friday, August 7, 2015 by Olivier Boudeville
 (olivier.boudeville@esperide.com)
*/


#include <stdio.h>
#include <stdlib.h>

#include "hdf5.h"

#include "erl_nif.h"

#include "dbg.h"

// For cell_type and all:
#include "erlhdf5.h"


//  H5D: Dataset Interface, dataset access and manipulation routines.


// Signatures:

static herr_t convert_space_status( H5D_space_status_t,  char* ) ;



// Converts space status into a string.
static herr_t convert_space_status( H5D_space_status_t space_status,
  char* space_status_str )
{
  if(space_status == H5D_SPACE_STATUS_ALLOCATED)
	strcpy(space_status_str, "H5D_SPACE_STATUS_ALLOCATED");
  else if(space_status == H5D_SPACE_STATUS_NOT_ALLOCATED)
	strcpy(space_status_str, "H5D_SPACE_STATUS_NOT_ALLOCATED");
  else if(space_status == H5D_SPACE_STATUS_PART_ALLOCATED)
	strcpy(space_status_str, "H5D_SPACE_STATUS_PART_ALLOCATED");
  else
	sentinel("Unknown status %d", space_status);

  return 0;

 error:
  return -1;
};



// Creates a new simple dataset, and opens it for access.
ERL_NIF_TERM h5dcreate( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  ERL_NIF_TERM ret;
  Handle* dcpl_res;
  char ds_name[ MAXBUFLEN ];
  hid_t file_id;
  hid_t type_id;
  hid_t dataspace_id;
  hid_t ds_id;
  hid_t dcpl_id;

  // Parses arguments:
  check( argc == 5, "Incorrect number of arguments");

  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get file resource from argv" ) ;

  check( enif_get_string( env, argv[1], ds_name, sizeof(ds_name),
	  ERL_NIF_LATIN1), "Cannot get dataset name from argv" ) ;

  check( enif_get_int( env, argv[2], &type_id ),
	"Cannot get datatype resource from argv" ) ;

  check( enif_get_int( env, argv[3], &dataspace_id ),
	"Cannot get dataspace resource from argv" ) ;

  check( enif_get_resource( env, argv[4], resource_type, (void**) &dcpl_res ),
	"Cannot get properties resource from argv" ) ;

  dcpl_id = dcpl_res->id ;

  // Creates a new file, using default properties:
  ds_id = H5Dcreate( file_id, ds_name, type_id, dataspace_id,
	/* Link creation property list */ H5P_DEFAULT, dcpl_id,
	/* Dataset access property list */ H5P_DEFAULT ) ;

  check( ds_id > 0, "Failed to create dataset." ) ;

  ret = enif_make_int( env, ds_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if( ds_id )
	H5Dclose( ds_id ) ;

  return error_tuple( env, "Cannot create dataset" ) ;

}



// Opens from specified file an existing dataset.
ERL_NIF_TERM h5dopen( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  ERL_NIF_TERM ret ;
  char ds_name[ MAXBUFLEN ] ;
  hid_t file_id ;
  hid_t ds_id ;

  // Parses arguments:
  check( argc == 2, "Incorrect number of arguments" ) ;
  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get file resource from argv" ) ;

  check( enif_get_string( env, argv[1], ds_name, sizeof( ds_name ),
	  ERL_NIF_LATIN1 ), "Cannot get dataset name from argv" ) ;

  // Creates a new file handle, using default properties:
  ds_id = H5Dopen( file_id, ds_name, H5P_DEFAULT ) ;
  check( ds_id > 0, "Failed to open dataset." ) ;

  ret = enif_make_int( env, ds_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if( ds_id )
	H5Dclose( ds_id ) ;
  return error_tuple( env, "Cannot open dataset" ) ;

}



// Closes specified dataset.
ERL_NIF_TERM h5dclose( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  hid_t ds_id ;

  // Parse arguments:
  check( argc == 1, "Incorrect number of arguments" ) ;
  check( enif_get_int( env, argv[0], &ds_id ),
	"Cannot get dataset handle from argv" ) ;

  check( ! H5Dclose( ds_id ), "Failed to close dataset.") ;
  return atom_ok ;

 error:
  return error_tuple( env, "Cannot close dataset" ) ;

}



// Determines whether space has been allocated for specified dataset.
ERL_NIF_TERM h5d_get_space_status( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  ERL_NIF_TERM ret ;
  hid_t ds_id ;
  H5D_space_status_t space_status ;
  char space_status_str[ MAXBUFLEN ] ;

  // Parses arguments:
  check( argc == 1, "Incorrect number of arguments" ) ;

  check( enif_get_int( env, argv[0], &ds_id ),
	"Cannot get dataset resource from argv" ) ;

  check( !H5Dget_space_status( ds_id, &space_status ),
	"Failed to get space status." ) ;

  // Converts code to string representation:
  check( ! convert_space_status( space_status, space_status_str) ,
	"Failed to convert space status %d", space_status ) ;

  // Makes an atom out of the string representation:
  ret = enif_make_atom( env, space_status_str ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if ( ds_id )
	H5Dclose( ds_id ) ;

  return error_tuple( env, "Cannot get dataspace status" ) ;

}



// Returns the amount of storage allocated for specified dataset.
ERL_NIF_TERM h5d_get_storage_size( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  ERL_NIF_TERM ret;
  hid_t ds_id;
  hsize_t size;

  // Parses arguments:

  check( argc == 1, "Incorrect number of arguments" ) ;

  check( enif_get_int( env, argv[0], &ds_id ),
	"Cannot get dataset resource from argv" ) ;

  size = H5Dget_storage_size( ds_id ) ;

  ret = enif_make_int64( env, size ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  return error_tuple( env, "Cannot determine storage size" ) ;

}



// Returns an identifier for a copy of the datatype for a dataset.
ERL_NIF_TERM h5dget_type( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Parses arguments:
  check( argc == 1, "Incorrect number of arguments" ) ;

  hid_t ds_id ;

  check( enif_get_int(env, argv[0], &ds_id ),
	"Cannot get dataset resource from argv" ) ;

  hid_t type_id = H5Dget_type( ds_id ) ;

  ERL_NIF_TERM ret = enif_make_int( env, type_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  return error_tuple( env, "Cannot determine storage size" ) ;

}



/*
 * Writes specified dataset into file.
 *
 * Note that we expect to write the full content of the dataset at once (no
 * specific size is specified here).
 *
 * Two types of cells are supported: native integer or float.
 *
 */
ERL_NIF_TERM h5dwrite( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Parses arguments:
  if ( argc != 2 )
	  return error_tuple( env, "Incorrect number of arguments" ) ;

  hid_t dataset_id ;

  if ( ! enif_get_int( env, argv[0], &dataset_id ) )
	return error_tuple( env, "Cannot get dataset handle from argv" ) ;

  ERL_NIF_TERM tuple_list = argv[1] ;

  // The input array will be of type, in C: int[U][V] our double[U][V].

  unsigned int list_length ;

  /*
	Gets the dimensions of input list : length of list is U (i.e. there are U
	data tuples), and size of tuples is V. All tuples are expected to be of the
	same size (same number of elements, V) and homogeneous (all their elements
	are of the same type, typically native int or float).
   */
  if ( ! enif_get_list_length( env, tuple_list, &list_length ) )
	return error_tuple( env, "Cannot get length of input list." ) ;

  printf( "List length (U): %u\n", list_length ) ;

  if ( list_length == 0 )
	return error_tuple( env, "Empty input list" ) ;

  /*
   * Determines once for all the size of tuples (i.e. V), based on the first
   * element found:
   *
   * Note: we could/should check that all tuples have the same size and that
   * their elements are all of the same type.
   *
   */

  ERL_NIF_TERM head, tail ;

  if ( ! enif_get_list_cell( env, tuple_list, &head, &tail ) )
	return error_tuple( env, "Cannot get first element of input list." ) ;

  const ERL_NIF_TERM *terms ;
  int tuple_size ;

  if ( ! enif_get_tuple( env, head, &tuple_size, &terms ) )
	  return error_tuple( env,
		"Cannot get the size of the first tuple from the input list" ) ;

  printf( "Tuple size (V): %u\n", tuple_size ) ;

  /*
   * Let's determine now the type of the cell elements, by assuming it is first
   * a floating-point value (a double), then a (native) integer:
   *
   */
  cell_type detected_type ;

  double floating_point_value ;
  int integer_value ;

  if ( enif_get_double( env, terms[0], &floating_point_value ) )
  {

	printf( "Detected float (double) %f.\n", floating_point_value ) ;
	detected_type = FLOAT ;

  }
  else if ( enif_get_int( env, terms[0], &integer_value ) )
  {

	printf( "Detected integer %i.\n", integer_value ) ;
	detected_type = INTEGER ;

  }
  else
  {

	return error_tuple( env, "Unsupported cell type." ) ;

  }


  /*
   * Allocates space for the intermediate array used to feed HDF:
   * (on the heap rather than on the stack, as can be big)
   */

  switch( detected_type )
  {

  case FLOAT:
	return write_float_array( dataset_id, env, list_length, tuple_size,
	  tuple_list ) ;
	break ;

  case INTEGER:
	return write_int_array( dataset_id, env, list_length, tuple_size,
	  tuple_list ) ;
	break ;

  default:
	return error_tuple( env, "Unsupported type for writing." ) ;

  }

}



// Returns an identifier for a copy of the dataspace for a dataset.
ERL_NIF_TERM h5dget_space( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{
  ERL_NIF_TERM ret;
  hid_t space_id, dataset_id;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &dataset_id), "cannot get dataset resource from argv");

  space_id = H5Dget_space(dataset_id);
  check(space_id >= 0, "Failed to get space.");

  ret = enif_make_int(env, space_id);
  return enif_make_tuple2(env, atom_ok, ret);

 error:
  return error_tuple(env, "cannot get space id");
};
