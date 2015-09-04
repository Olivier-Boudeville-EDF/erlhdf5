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

#include "erlhdf5.h"


int convert_array_to_nif_array( ErlNifEnv* env, hsize_t size, hsize_t *arr_from,
  ERL_NIF_TERM* arr_to )
{

  int i;

  for( i = 0; i < size; i++ )
  {
	arr_to[i] = enif_make_int( env, arr_from[i] ) ;
  }

  return 0 ;

}



int convert_int_array_to_nif_array( ErlNifEnv* env, hsize_t size, int *arr_from,
  ERL_NIF_TERM* arr_to )
{

  int i;

  for( i = 0; i < size; i++ )
  {
	arr_to[i] = enif_make_int( env, arr_from[i] ) ;
  }

  return 0 ;

}



int convert_nif_to_hsize_array( ErlNifEnv* env, hsize_t size,
  const ERL_NIF_TERM* arr_from, hsize_t *arr_to )
{

  int n, i ;

  for( i = 0; i < size; i++ )
  {
	check( enif_get_int( env, arr_from[i], &n ),
	  "Error getting array element " ) ;

	arr_to[i] = (hsize_t) n ;

  }

  return 0;

 error:
  return -1 ;

}


// Helper to translate [ tuple( double() ) ] into a C-array for HDF5.
ERL_NIF_TERM write_float_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list )
{

  double * buffer_for_hdf = enif_alloc(
	list_length * tuple_size * sizeof( double ) ) ;

  if ( ! buffer_for_hdf )
	return error_tuple( env, "Cannot allocate intermediate array memory" ) ;

  // Flat index (as if the array was monodimensional):
  unsigned int global = 0 ;

  ERL_NIF_TERM head, tail ;

  const ERL_NIF_TERM *terms ;

  // Go through each list element, and unpack it:
  while ( enif_get_list_cell( env, tuple_list, &head, &tail ) )
  {

	int this_tuple_size ;

	if ( ! enif_get_tuple( env, head, &this_tuple_size, &terms ) )
	{
	  enif_free( buffer_for_hdf ) ;
	  return error_tuple( env, "Cannot get tuples from the input list" ) ;
	}

	//printf( "Found tuple size: %u\n", this_tuple_size ) ;

	if ( this_tuple_size != tuple_size )
	{
	  enif_free( buffer_for_hdf ) ;
	  return error_tuple( env, "Non-uniform tuple size detected" ) ;
	}

	unsigned int j ;

	// Iterates through the elements of this tuple and stores them:
	for( j = 0; j < tuple_size; j++ )
	{

	  if ( ! enif_get_double( env, terms[j], buffer_for_hdf + global ) )
	  {
		enif_free( buffer_for_hdf ) ;
		return error_tuple( env, "Cell does not contain a double value" ) ;
	  }

	  //printf( "element =%i\n", *( buffer_for_hdf + global ) );

	  global++ ;

	}

	tuple_list = tail ;

  }

  // Finally writes the translated data to the dataset:
  if( H5Dwrite(
	  /* target */ dataset_id,
	  /* cell type */ H5T_NATIVE_DOUBLE,
	  /* memory and selection dataspace */ H5S_ALL,
	  /* selection within the file dataset's dataspace */ H5S_ALL,
	  /* default data transfer properties */ H5P_DEFAULT,
	  /* source location */ buffer_for_hdf ) < 0 )
  {
	enif_free( buffer_for_hdf ) ;
	return error_tuple( env, "Failed to write into dataset" ) ;
  }

  enif_free( buffer_for_hdf ) ;

  return atom_ok ;

}



// Helper to translate [ tuple( integer() ) ] into a C-array for HDF5.
ERL_NIF_TERM write_int_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list )
{

  int * buffer_for_hdf = enif_alloc(
	list_length * tuple_size * sizeof( int ) ) ;

  if ( ! buffer_for_hdf )
	return error_tuple( env, "Cannot allocate intermediate array memory" ) ;

  // Flat index (as if the array was monodimensional):
  unsigned int global = 0 ;

  ERL_NIF_TERM head, tail ;

  const ERL_NIF_TERM *terms ;

  // Goes through each list element, and unpack it:
  while ( enif_get_list_cell( env, tuple_list, &head, &tail ) )
  {

	int this_tuple_size ;

	if ( ! enif_get_tuple( env, head, &this_tuple_size, &terms ) )
	{
	  enif_free( buffer_for_hdf ) ;
	  return error_tuple( env, "Cannot get tuples from the input list" ) ;
	}

	//printf( "Found tuple size: %u\n", this_tuple_size ) ;

	if ( this_tuple_size != tuple_size )
	{
	  enif_free( buffer_for_hdf ) ;
	  return error_tuple( env, "Non-uniform tuple size detected" ) ;
	}

	unsigned int j ;

	// Iterates through the elements of this tuple and stores them:
	for( j = 0; j < tuple_size; j++ )
	{

	  if ( ! enif_get_int( env, terms[j], buffer_for_hdf + global ) )
	  {
		enif_free( buffer_for_hdf ) ;
		return error_tuple( env, "Cell does not contain an integer" ) ;
	  }

	  //printf( "element =%i\n", *( buffer_for_hdf + global ) );

	  global++ ;

	}

	tuple_list = tail ;

  }

  // Finally writes the translated data to the dataset:
  if( H5Dwrite( dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	  buffer_for_hdf ) < 0 )
  {
	enif_free( buffer_for_hdf ) ;
	return error_tuple( env, "Failed to write into dataset" ) ;
  }

  enif_free( buffer_for_hdf ) ;

  return atom_ok ;

}
