
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



// Describes data, typically to be written in a dataset.
struct DataDescriptor
{

  // Number of dimensions of the data:
  unsigned int dimension_count ;

  // Type of an atomic element in the data:
  cell_type type ;

  // List length (number of tuples):
  unsigned int len ;

  // Size of each tuple:
  int size ;

  // Not allowed in C: ERL_NIF_TERM * error_term = NULL ;
  ERL_NIF_TERM error_term ;

} ;


// 0, 1:
typedef enum { false, true } bool ;


// Forward declarations:
bool detect_type( ERL_NIF_TERM data_list, ErlNifEnv* env,
  struct DataDescriptor * detected_desc ) ;

cell_type detect_cell_type( ERL_NIF_TERM term, ErlNifEnv* env ) ;


// Converts space status into a string.
static herr_t convert_space_status( H5D_space_status_t space_status,
  char* space_status_str )
{
  if ( space_status == H5D_SPACE_STATUS_ALLOCATED )
	strcpy( space_status_str, "H5D_SPACE_STATUS_ALLOCATED" ) ;
  else if ( space_status == H5D_SPACE_STATUS_NOT_ALLOCATED )
	strcpy( space_status_str, "H5D_SPACE_STATUS_NOT_ALLOCATED" ) ;
  else if ( space_status == H5D_SPACE_STATUS_PART_ALLOCATED )
	strcpy( space_status_str, "H5D_SPACE_STATUS_PART_ALLOCATED" ) ;
  else
	sentinel( "Unknown status %d", space_status ) ;

  return 0 ;

 error:
  return -1 ;

}



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



/*
 * Opens from specified file an existing dataset.
 *
 * This corresponds to h5dopen/{2,3}, depending on whether an access property
 * list is specified:
 *
 * -spec h5dopen( HDF5File::file_handle(), DatasetName::string() ) ->
 *   { 'ok', dataset_handle() } | error().
 *
 * and
 *
 * -spec h5dopen( HDF5File::file_handle(), DatasetName::string(),
 *   DatasetAccessPropList ) -> { 'ok', dataset_handle() } | error().
 *
 */
ERL_NIF_TERM h5dopen( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Dataset access property list:
  hid_t ds_proplist ;

   switch ( argc )
  {

  case 2:
	ds_proplist = H5P_DEFAULT ;
	break ;

  case 3:
	{

	  /*
	   * Actually an atom like 'H5P_DATASET_ACCESS' should be specified here
	   * (h5dopen/3 not functional)
	   *
	   */
	  hid_t class_id ;
	  if( ! enif_get_int( env, argv[2], &class_id ) )
		return error_tuple( env,
		  "Cannot get dataset property list from argv" ) ;

	  ds_proplist = H5Pcreate( class_id ) ;
	}
	break ;

  default:
	return error_tuple( env, "Incorrect number of arguments" ) ;

  }

  hid_t file_id ;

  if ( ! enif_get_int( env, argv[0], &file_id ) )
	return error_tuple( env, "Cannot get file resource from argv" ) ;

  char ds_name[ MAXBUFLEN ] ;
  if ( ! enif_get_string( env, argv[1], ds_name, sizeof( ds_name ),
	  ERL_NIF_LATIN1 ) )
	return error_tuple( env, "Cannot get dataset name from argv" ) ;

  // Creates a new file handle, using default properties:
  hid_t ds_id = H5Dopen( file_id, ds_name, ds_proplist ) ;
  if ( ds_id <= 0 )
	return error_tuple( env, "Failed to open dataset" ) ;

  ERL_NIF_TERM ret = enif_make_int( env, ds_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

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
 * Corresponds to h5dwrite/2, i.e. a writing of the data on the full dataset of
 * the file (hence the sizes of the data and of the dataset must exactly match):
 *
 * -spec h5dwrite_2( dataset_handle(), data() ) -> 'ok' | error().
 *
 */
ERL_NIF_TERM h5dwrite_2( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Parses the two arguments:

  hid_t dataset_id ;

  if ( ! enif_get_int( env, argv[0], &dataset_id ) )
	return error_tuple( env, "Cannot get dataset handle from argv" ) ;

  ERL_NIF_TERM data_list = argv[1] ;

  struct DataDescriptor detected_desc ;

  if ( ! detect_type( data_list, env, &detected_desc )  )
	return detected_desc.error_term ;

  /*
   * Allocates space for the intermediate array used to feed HDF:
   * (on the heap rather than on the stack, as it can be big)
   */

  // Clearer than a switch, as needing blocks:
  if ( detected_desc.dimension_count == 1 )
  {

	switch( detected_desc.type )
	{

	case INTEGER:
	  return write_ints_to_array( dataset_id, env, detected_desc.len,
		data_list, /* using the full file dataspace */ H5S_ALL ) ;
	  break ;

	case FLOAT:
	  return write_floats_to_array( dataset_id, env, detected_desc.len,
		data_list, /* using the full file dataspace */ H5S_ALL ) ;
	  break ;

	default:
	  return error_tuple( env, "Unsupported datatype for 1D writing" ) ;

	}

  }
  else if ( detected_desc.dimension_count == 2 )
  {

	switch( detected_desc.type )
	{

	case INTEGER:
	  return write_int_tuples_to_array( dataset_id, env, detected_desc.len,
		detected_desc.size, data_list,
		/* using the full file dataspace */ H5S_ALL ) ;
	  break ;

	case FLOAT:
	  return write_float_tuples_to_array( dataset_id, env, detected_desc.len,
		detected_desc.size, data_list,
		/* using the full file dataspace */ H5S_ALL ) ;
	  break ;

	default:
	  return error_tuple( env, "Unsupported datatype for 2D writing" ) ;

	}

  }
  else
  {

	return error_tuple( env, "Unsupported datatype dimension for writing" ) ;

  }

}



/*
 * Corresponds to h5dwrite/3, i.e a writing making use of a dataspace-based
 * selection on the target file:
 *
 * Like h5dwrite/2, except that a target dataspace is used, instead of using the
 * full file dataspace.
 *
 * -spec h5dwrite_3( dataset_handle(), dataspace_handle(), data() ) ->
 *					  'ok' | error().
 *
 */
ERL_NIF_TERM h5dwrite_3( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Parses the three arguments:

  hid_t dataset_id ;

  if ( ! enif_get_int( env, argv[0], &dataset_id ) )
	return error_tuple( env, "Cannot get dataset handle from argv" ) ;

  hid_t dataspace_id ;

  if ( ! enif_get_int( env, argv[1], &dataspace_id ) )
	return error_tuple( env, "Cannot get dataspace handle from argv" ) ;

  ERL_NIF_TERM data_list = argv[2] ;

  struct DataDescriptor detected_desc ;

  if ( ! detect_type( data_list, env, &detected_desc ) )
	return detected_desc.error_term ;

  /*
   * Allocates space for the intermediate array used to feed HDF:
   * (on the heap rather than on the stack, as can be big)
   */


  // Clearer than a switch, as needing blocks:
  if ( detected_desc.dimension_count == 1 )
  {

	switch( detected_desc.type )
	{

	case INTEGER:
	  return write_ints_to_array( dataset_id, env, detected_desc.len,
		data_list, dataspace_id ) ;
	  break ;

	case FLOAT:
	  return write_floats_to_array( dataset_id, env, detected_desc.len,
		data_list, dataspace_id ) ;
	  break ;

	default:
	  return error_tuple( env, "Unsupported datatype for 1D writing" ) ;

	}

  }
  else if ( detected_desc.dimension_count == 2 )
  {

	switch( detected_desc.type )
	{

	case INTEGER:
	  return write_int_tuples_to_array( dataset_id, env, detected_desc.len,
		detected_desc.size, data_list, dataspace_id ) ;
	  break ;

	case FLOAT:
	  return write_float_tuples_to_array( dataset_id, env, detected_desc.len,
		detected_desc.size, data_list, dataspace_id ) ;
	  break ;

	default:
	  return error_tuple( env, "Unsupported datatype for 2D writing" ) ;

	}

  }
  else
  {

	return error_tuple( env, "Unsupported datatype dimension for writing" ) ;

  }

}



/*
 * Detects the dimension (in [1,2]) and type (in [ 'INTEGER', 'FLOAT' ]) of the
 * elements aggregated in the specified data list.
 *
 * This list is assumed to contain either directly atomic, homogeneous elements,
 * or tuples of all the same size, whose elements have all the same type (hence
 * checking the first element of the first tuple is sufficient).
 *
 * Said differently, the data_list is either [ T ] or [ tuple(T) ], with T ::
 * integer() | float().
 *
 * Returns whether the execution is a success, and fills specified data
 * descriptor.
 *
 * (helper)
 *
 */
bool detect_type( ERL_NIF_TERM data_list, ErlNifEnv* env,
  struct DataDescriptor* detected_desc )
{

  /*
   * The input array will be of type, in C, in: int[U], double[U], int[U][V] or
   * double[U][V].
   *
   */

  /*
   * Gets the dimensions of input data: length of list is U (i.e. there are U
   * data elements), and size of tuples is V (possibly 1, if no tuple at all).
   *
   * All tuples are expected to be of the same size (same number of elements, V)
   * and homogeneous (all their elements are of the same type, typically native
   * int or double).
   *
   */

  if ( ! enif_get_list_length( env, data_list, &(detected_desc->len) ) )
  {

	// Probably not a list then:
	detected_desc->error_term = error_tuple( env,
	  "Cannot get length of input list" ) ;

	return false ;

  }

  //printf( "List length (U): %u\n", &detected_desc.len ) ;

  if ( detected_desc->len == 0 )
  {

	detected_desc->error_term = error_tuple( env, "Empty input list" ) ;

	return false ;

  }


  /*
   * Determines once for all the size of tuples (i.e. V), based on the first
   * element found:
   *
   * Note: we could/should check that all tuples have the same size and that
   * their elements are all of the same type (better done optionally and
   * Erlang-level)
   *
   */

  ERL_NIF_TERM head, tail ;

  // Unlikely to fail:
  if ( ! enif_get_list_cell( env, data_list, &head, &tail ) )
  {

	 detected_desc->error_term = error_tuple( env,
	   "Cannot get first element of input list." ) ;

	 return false ;

  }


  const ERL_NIF_TERM * terms ;

  if ( ! enif_get_tuple( env, head, &(detected_desc->size), &terms ) )
  {

	/*
	 * No tuple found at first position, hence here we should have a
	 * mono-dimensional list of (atomic) elements:
	 *
	 */

	detected_desc->dimension_count = 1 ;

	detected_desc->type = detect_cell_type( head, env ) ;

	if ( detected_desc->type == UNKNOWN_TYPE )
	{

	  detected_desc->error_term = error_tuple( env,
		"Unknown cell type detected" ) ;

	  return false ;

	}

	// detected_desc->len already set.
	detected_desc->size = 1 ;

	return true ;

  }

  /*
   * Here we found a tuple, are in dimension at least two, we suppose exactly
   * two:
   *
   */
  detected_desc->dimension_count = 2 ;

  detected_desc->type = detect_cell_type( terms[0], env ) ;

  // All fields filled.

  // No error here:
  return true ;

}



// Returns the type of specified cell.
cell_type detect_cell_type( ERL_NIF_TERM term, ErlNifEnv* env )
{

  /*
   * Let's determine now the type of the cell elements, by assuming it is first
   * a floating-point value (a native double), then a (native) integer:
   *
   */

  int integer_value ;
  double floating_point_value ;

  if ( enif_get_int( env, term, &integer_value ) )
  {

	//printf( "Detected integer %i.\n", integer_value ) ;
	return INTEGER ;

  }
  else if ( enif_get_double( env, term, &floating_point_value ) )
  {

	//printf( "Detected float (double) %f.\n", floating_point_value ) ;
	return FLOAT ;

  }
  else
  {

	 return UNKNOWN_TYPE ;

  }

}



/*
 * Writes specified dataset into file, using a target dataspace if specified.
 *
 * Note that, if using the version of arity 2, we expect to write the full
 * content of the dataset at once (no specific size is specified here), while
 * the version of arity 3 the specified dataspace allows to select the parts of
 * the target dataset that will be written (updated).
 *
 * Two types of cells are supported: native integer or float.
 *
 */
ERL_NIF_TERM h5dwrite( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  /*
   * There are two versions, h5dwrite/{2,3}:
   *
   */

  switch ( argc )
  {

  case 2:
	return h5dwrite_2( env, argc, argv ) ;
	break ;

  case 3:
	return h5dwrite_3( env, argc, argv ) ;
	break ;

  default:
	return error_tuple( env, "Invalid arity for h5dwrite" ) ;

  }

}



// Returns an identifier for a copy of the dataspace for a dataset.
ERL_NIF_TERM h5dget_space( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  // Parses arguments
  check( argc == 1, "Incorrect number of arguments" ) ;

  hid_t dataset_id ;

  check( enif_get_int( env, argv[0], &dataset_id ),
	"Cannot get dataset resource from argv" ) ;

  hid_t space_id = H5Dget_space( dataset_id ) ;

  check( space_id >= 0, "Failed to get space." ) ;

  ERL_NIF_TERM ret = enif_make_int( env, space_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  return error_tuple( env, "Cannot get space id" ) ;

}
