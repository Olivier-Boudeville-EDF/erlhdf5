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
/* License along with erlhdf5.  If not, see */
/* <http://www.gnu.org/licenses/>. */

/* Author contact: romanshestakov@yahoo.co.uk */

/*
 Forked on Thursday, August 6, 2015 by Olivier Boudeville
 (olivier.boudeville@esperide.com)
*/



#include <stdio.h>
#include <stdlib.h>

#include "hdf5.h"
#include "hdf5_hl.h"

#include "erl_nif.h"
#include "dbg.h"
#include "erlhdf5.h"


// H5LT


// Forward declarations:

static int unpack_int_list( ErlNifEnv* env, ERL_NIF_TERM* list, int* data ) ;


// Creates a new simple dataspace, and opens it for access:
ERL_NIF_TERM h5lt_make_dataset( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{
  hid_t file_id;
  char ds_name[MAXBUFLEN];
  const ERL_NIF_TERM *dims;
  int* data;
  int rank, arity;
  hsize_t* dims_arr; // array specifiying the size of each dimension
  ERL_NIF_TERM list;
  unsigned int list_length;

  // parse arguments
  check(argc == 5, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &file_id ), "cannot get file id from argv");
  check(enif_get_string(env, argv[1], ds_name, sizeof(ds_name), ERL_NIF_LATIN1), "cannot get dataset name from argv");
  check(enif_get_int(env, argv[2], &rank ), "cannot get rank from argv");
  check(enif_get_tuple(env, argv[3], &arity, &dims), "cannot get dimensions from argv");
  check(enif_get_list_length(env, argv[4], &list_length), "empty data");
  list = argv[4];

  // allocate array of size rank
  dims_arr = (hsize_t*) enif_alloc(arity * sizeof(hsize_t));
  check(!convert_nif_to_hsize_array(env, arity, dims, dims_arr), "cannot convert dims arr");

  // allocate space for array to hold elements of list
  data = enif_alloc(list_length * sizeof(int));
  check(data, "cannot allocate mem");

  // convert a list of ints into array
  check(!unpack_int_list(env, &list, data), "cannot unpack list");

  // make a dataset
  check(!H5LTmake_dataset(file_id, ds_name, arity, dims_arr, H5T_NATIVE_INT, data), "Failed to make dataset.");

  // cleanup
  enif_free(dims_arr);
  enif_free(data);
  return atom_ok;

 error:
   if(dims_arr) enif_free(dims_arr);
   if(data) enif_free(data);
  return error_tuple(env, "Cannot make dataset");

}


// unpack a list of ints into an array of ints
static int unpack_int_list(ErlNifEnv* env, ERL_NIF_TERM* list, int* data)
{
  int i = 0;
  ERL_NIF_TERM head, tail;

  while(enif_get_list_cell(env, *list, &head, &tail)) {
	check(enif_get_int(env, head, &data[i++]), "error upacking an element");
	*list = tail;
  }
  return 0;

 error:
  return -1;
}



/*
 * Reads specified native integer dataset from specified file.
 *
 * -spec h5lt_read_dataset_int( file_handle(), DatasetName::string() ) ->
 *                 { 'ok', [ integer() ] } | error().
 *
 */
ERL_NIF_TERM h5lt_read_dataset_int( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 2, "Incorrect number of arguments" ) ;

  hid_t file_id ;
  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get file handle from argv" ) ;


  char ds_name[ MAXBUFLEN ] ;

  check( enif_get_string( env, argv[1], ds_name, sizeof(ds_name),
	  ERL_NIF_LATIN1 ), "Cannot get the name of dataset from argv" ) ;

  // Get the dimensions of this dataset:
  int ndims ;

  check( ! H5LTget_dataset_ndims( file_id, ds_name, &ndims ),
	"Failed to determine dataspace dimensions." ) ;

  // Get dataset information:
  hsize_t * dims = enif_alloc( ndims * sizeof( hsize_t ) ) ;

  check( ! H5LTget_dataset_info( file_id, ds_name, dims, NULL, NULL),
	"Failed to get information about dataset." ) ;

  // Finds out the total number of values in the dataset from the dimensions:
  hsize_t n_values = 1 ;

  int i ;

  for( i = 0; i < ndims; i++ )
  {
	n_values = n_values * dims[i] ;
  }

  // Allocates the corresponding space to hold the dataset values:
  int * data = enif_alloc( n_values * sizeof( int ) ) ;

  // Reads that dataset:
  check( ! H5LTread_dataset_int( file_id, ds_name, data ),
	"Failed to read dataset." ) ;

  // Converts the array of ints into a nif array:
  ERL_NIF_TERM * data_arr = (ERL_NIF_TERM*) enif_alloc(
	sizeof( ERL_NIF_TERM ) * n_values ) ;

  check( ! convert_int_array_to_nif_array( env, n_values, data, data_arr ),
	"Cannot convert array to nif" ) ;

  // Makes a list of ERL_NIF_TERM, to return to the caller:
  ERL_NIF_TERM ret = enif_make_list_from_array( env, data_arr, n_values ) ;

  // Cleanup:
  enif_free( dims ) ;
  enif_free( data ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if ( dims )
	enif_free( dims ) ;

  if ( data )
	enif_free( data ) ;

  return error_tuple( env, "Cannot read integer dataset" ) ;

}



/*
 * Reads specified native double (floating-point) dataset from specified file.
 *
 * -spec h5lt_read_dataset_double( file_handle(), DatasetName::string() ) ->
 *                 { 'ok', [ float() ] } | error().
 *
 */
ERL_NIF_TERM h5lt_read_dataset_double( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 2, "Incorrect number of arguments" ) ;

  hid_t file_id ;
  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get file handle from argv" ) ;


  char ds_name[ MAXBUFLEN ] ;

  check( enif_get_string( env, argv[1], ds_name, sizeof(ds_name),
	  ERL_NIF_LATIN1 ), "Cannot get the name of dataset from argv" ) ;

  // Get the dimensions of this dataset:
  int ndims ;

  check( ! H5LTget_dataset_ndims( file_id, ds_name, &ndims ),
	"Failed to determine dataspace dimensions." ) ;

  // Get dataset information:
  hsize_t * dims = enif_alloc( ndims * sizeof( hsize_t ) ) ;

  check( ! H5LTget_dataset_info( file_id, ds_name, dims, NULL, NULL ),
	"Failed to get information about dataset." ) ;

  // Finds out the total number of values in the dataset from the dimensions:
  hsize_t n_values = 1 ;

  int i ;

  for ( i = 0; i < ndims; i++ )
  {
	n_values = n_values * dims[i] ;
  }

  //printf( "Allocating space for %llu doubles.", n_values ) ;

  // Allocates the corresponding space to hold the dataset values:
  double * data = enif_alloc( n_values * sizeof( double ) ) ;

  check( data != NULL, "Buffer allocation failed" ) ;

   // Reads that dataset:
  check( ! H5LTread_dataset_double( file_id, ds_name, data ),
	"Failed to read dataset." ) ;

  // Converts the array of ints into a nif array:
  ERL_NIF_TERM * data_arr = (ERL_NIF_TERM*) enif_alloc(
	sizeof( ERL_NIF_TERM ) * n_values ) ;

  check( ! convert_double_array_to_nif_array( env, n_values, data, data_arr ),
	"Cannot convert array to nif" ) ;

  // Makes a list of ERL_NIF_TERM, to return to the caller:
  ERL_NIF_TERM ret = enif_make_list_from_array( env, data_arr, n_values ) ;

  // Cleanup:
  enif_free( dims ) ;
  enif_free( data ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if ( dims )
	enif_free( dims ) ;

  if ( data )
	enif_free( data ) ;

  return error_tuple( env, "Cannot read double dataset" ) ;

}



/*
 * Reads specified native string dataset from specified file.
 *
 * -spec h5lt_read_dataset_string( file_handle(), DatasetName::string() ) ->
 *                 { 'ok', [ string() ] } | error().
 *
 */
ERL_NIF_TERM h5lt_read_dataset_string( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 2, "Incorrect number of arguments" ) ;

  hid_t file_id ;
  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get file handle from argv" ) ;

  char ds_name[ MAXBUFLEN ] ;

  check( enif_get_string( env, argv[1], ds_name, sizeof(ds_name),
	  ERL_NIF_LATIN1 ), "Cannot get the name of dataset from argv" ) ;

  // Get the dimensions of this dataset:
  int ndims ;

  check( ! H5LTget_dataset_ndims( file_id, ds_name, &ndims ),
	"Failed to determine dataspace dimensions." ) ;

  //printf( "ndims: %i\n", ndims ) ;

  // Get dataset information:
  hsize_t * dims = enif_alloc( ndims * sizeof( hsize_t ) ) ;

  H5T_class_t class_id ;

  //size_t cell_size ;

  check( ! H5LTget_dataset_info( file_id, ds_name, dims, &class_id,
	  NULL /* &cell_size */ ), "Failed to get information about dataset." ) ;

  // 3 is H5T_STRING: printf( "class: %i\n", class_id ) ;

  check( class_id == H5T_STRING, "Not a dataset string" ) ;

  // size_of( char * ): printf( "type size: %lu\n", cell_size ) ;

  // Finds out the total number of values in the dataset from the dimensions:
  hsize_t n_values = 1 ;

  int i ;

  for ( i = 0; i < ndims; i++ )
  {
	n_values = n_values * dims[i] ;
  }

  //printf( "%llu strings will be retrieved.", n_values ) ;

  char ** string_arr = enif_alloc( sizeof(char * ) * n_values ) ;

  check( string_arr != NULL, "String buffer allocation failed" ) ;

   // Reads that dataset:
  check( ! H5LTread_dataset_string( file_id, ds_name, (char *) string_arr ),
	"Failed to read dataset." ) ;


  //for ( i=0; i < n_values; i++ )
  //	printf( "#%i: %s\n", i, string_arr[i] ) ;

  /*
   * Preparing the (flat) list that will contain them by creating its string
   * elements:
   */
  ERL_NIF_TERM * string_terms = enif_alloc(
	sizeof( ERL_NIF_TERM ) * n_values ) ;

  check( string_terms != NULL, "Term buffer allocation failed" ) ;

  for ( i=0; i < n_values; i++ )
	string_terms[i] = enif_make_string( env, string_arr[i], ERL_NIF_LATIN1 ) ;

  // Creates the list out of this term array:
  ERL_NIF_TERM string_list = enif_make_list_from_array( env, string_terms,
	n_values ) ;

  // Cleanup:
  enif_free( dims ) ;
  enif_free( string_arr ) ;
  enif_free( string_terms ) ;

  return enif_make_tuple2( env, atom_ok, string_list ) ;


 error:
  if ( dims )
	enif_free( dims ) ;

  if ( string_arr )
	enif_free( string_arr ) ;

  if ( string_terms )
	enif_free( string_terms ) ;

  return error_tuple( env, "Cannot read string dataset" ) ;

}



// Determines the dimensionality of a dataspace.
ERL_NIF_TERM h5ltget_dataset_ndims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{
  hid_t file_id;
  int ndims;
  char ds_name[MAXBUFLEN];

  // parse arguments
  check(argc == 2, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &file_id), "cannot get resource from argv");
  check(enif_get_string(env, argv[1], ds_name, sizeof(ds_name), ERL_NIF_LATIN1), "cannot get dataset name from argv");

  check(!H5LTget_dataset_ndims(file_id, ds_name, &ndims), "Failed to determine dataspace dimensions.");
  return enif_make_tuple2(env, atom_ok, enif_make_int(env, ndims));

 error:
  return error_tuple(env, "Failed to determine dataspace dimensions");
}



/*
 * Gets information about a dataset.
 *
 * -spec h5ltget_dataset_info( file_handle(), dataset_name(), rank() ) ->
 *           { ok, tuple( dimension() ) } | error().
 *
 */
ERL_NIF_TERM h5ltget_dataset_info( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 3, "Incorrect number of arguments" ) ;

  hid_t file_id ;
  check( enif_get_int( env, argv[0], &file_id ),
	"Cannot get resource from argv" ) ;

  char ds_name[ MAXBUFLEN ] ;
  check( enif_get_string( env, argv[1], ds_name, sizeof( ds_name ),
	  ERL_NIF_LATIN1 ), "Cannot get dataset name from argv" ) ;

  int rank ;
  check( enif_get_int( env, argv[2], &rank ), "Cannot get rank from argv" ) ;

  // Allocates the dimension array:
  hsize_t * dims = enif_alloc( rank * sizeof( hsize_t ) ) ;

  check( ! H5LTget_dataset_info( file_id, ds_name, dims, NULL, NULL ),
	"Failed to get info about dataset." ) ;

  // Allocates the array of ERL_NIF_TERM, for conversion:
  ERL_NIF_TERM * dims_arr = (ERL_NIF_TERM*) enif_alloc(
	sizeof( ERL_NIF_TERM ) * rank ) ;

  // We need a NIF array:
  check( ! convert_array_to_nif_array( env, rank, dims, dims_arr ),
	"Cannot convert array to nif" ) ;

  // Converts that dimension array into a tuple (previously was a list):
  ERL_NIF_TERM dims_list = enif_make_tuple_from_array( env, dims_arr, rank ) ;

  // Cleanup:
  enif_free( dims ) ;

  return enif_make_tuple2( env, atom_ok, dims_list ) ;

 error:
  if ( dims )
	enif_free( dims ) ;

  return error_tuple( env, "Failed to get info about dataset" ) ;

}
