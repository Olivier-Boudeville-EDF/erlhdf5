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



int convert_int_array_to_nif_array( ErlNifEnv* env, hsize_t size, int* arr_from,
  ERL_NIF_TERM* arr_to )
{

  int i ;

  for ( i = 0; i < size; i++ )
  {
	arr_to[i] = enif_make_int( env, arr_from[i] ) ;
  }

  return 0 ;

}



int convert_double_array_to_nif_array( ErlNifEnv* env, hsize_t size,
  double* arr_from, ERL_NIF_TERM* arr_to )
{

  int i ;

  for ( i = 0; i < size; i++ )
  {
	//printf( "read: double -> nif: %f / %e\n", arr_from[i], arr_from[i]);
	arr_to[i] = enif_make_double( env, arr_from[i] ) ;
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



/*
 * Helper to translate [ tuple( float() ) ] terms into a C-array of doubles for
 * injecting (writing) that (in-memory) data in specified (in-file) dataspace.
 *
 */
ERL_NIF_TERM write_float_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list,
  hid_t file_dataspace_id )
{

  //printf( "Writing now float array (len=%u, size=%i, elem_size=%lu)\n",
  //	list_length, tuple_size, sizeof( double ) ) ;

  hsize_t element_count = list_length * tuple_size ;

  double * buffer_for_hdf = (double *) enif_alloc( 
	element_count * sizeof( double ) ) ;

  if ( ! buffer_for_hdf )
	return error_tuple( env, "Cannot allocate intermediate array memory" ) ;

  // Flat index (as if the array was monodimensional):
  unsigned int global = 0 ;

  ERL_NIF_TERM head, tail ;

  const ERL_NIF_TERM * tuple_elements ;

  // Go through each list element (tuple), and unpack it:
  while ( enif_get_list_cell( env, tuple_list, &head, &tail ) )
  {

	int this_tuple_size ;

	if ( ! enif_get_tuple( env, head, &this_tuple_size, &tuple_elements ) )
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

	unsigned int i ;

	// Iterates through the elements of this tuple and stores them:
	for( i = 0; i < tuple_size; i++ )
	{

	  if ( ! enif_get_double( env, tuple_elements[i],
		  buffer_for_hdf + global ) )
	  {
		enif_free( buffer_for_hdf ) ;
		return error_tuple( env, "Cell does not contain a double value" ) ;
	  }

	  //printf( "write element =%f\n", *( buffer_for_hdf + global ) );

	  global++ ;

	}

	// Recurses:
	tuple_list = tail ;

  }

  // Just one dimension:
  hid_t mem_dataspace_id = H5Screate_simple( /* rank */ 1, &element_count,
	/* max dims */ NULL ) ;

  if ( mem_dataspace_id < 0 )
	return error_tuple( env, "Cannot create a memory dataspace" ) ;

  // Finally, writes the translated data to the dataset:
  if( H5Dwrite(
	  /* target */ dataset_id,
	  /* cell type */ H5T_NATIVE_DOUBLE,
	  /* memory and selection dataspace */ mem_dataspace_id,
	  /* selection within the file dataset's dataspace */ file_dataspace_id,
	  /* default data transfer properties */ H5P_DEFAULT,
	  /* source location */ buffer_for_hdf ) < 0 )
  {
	enif_free( buffer_for_hdf ) ;
	return error_tuple( env, "Failed to write into double dataset" ) ;
  }

  enif_free( buffer_for_hdf ) ;

  return atom_ok ;

}



/*
 * Helper to translate [ tuple( integer() ) ] terms into a C-array of ints for
 * injecting (writing) that (in-memory) data in specified (in-file) dataspace.
 *
 */
ERL_NIF_TERM write_int_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list,
  hid_t file_dataspace_id )
{

  hsize_t element_count = list_length * tuple_size ;

  int * buffer_for_hdf = enif_alloc( element_count * sizeof( int ) ) ;

  if ( ! buffer_for_hdf )
	return error_tuple( env, "Cannot allocate intermediate array memory" ) ;

  // Flat index (as if the array was monodimensional):
  unsigned int global = 0 ;

  ERL_NIF_TERM head, tail ;

  const ERL_NIF_TERM * tuple_elements ;

  // Goes through each list element, and unpacks it:
  while ( enif_get_list_cell( env, tuple_list, &head, &tail ) )
  {

	int this_tuple_size ;

	if ( ! enif_get_tuple( env, head, &this_tuple_size, &tuple_elements ) )
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

	unsigned int i ;

	// Iterates through the elements of this tuple and stores them:
	for( i = 0; i < tuple_size; i++ )
	{

	  if ( ! enif_get_int( env, tuple_elements[i], buffer_for_hdf + global ) )
	  {
		enif_free( buffer_for_hdf ) ;
		return error_tuple( env, "Cell does not contain an integer" ) ;
	  }

	  //printf( "element =%i\n", *( buffer_for_hdf + global ) ) ;

	  global++ ;

	}

	tuple_list = tail ;

  }

  // Just one dimension:
  hid_t mem_dataspace_id = H5Screate_simple( /* rank */ 1, &element_count,
	/* max dims */ NULL ) ;

  if ( mem_dataspace_id < 0 )
	return error_tuple( env, "Cannot create a memory dataspace" ) ;


  // Finally, writes the translated data to the dataset:
  if( H5Dwrite(
	  /* target */ dataset_id,
	  /* cell type */ H5T_NATIVE_INT,
	  /* memory and selection dataspace */ mem_dataspace_id,
	  /* selection within the file dataset's dataspace */ file_dataspace_id,
	  /* default data transfer properties */ H5P_DEFAULT,
	  /* source location */ buffer_for_hdf ) < 0 )

  {
	enif_free( buffer_for_hdf ) ;
	return error_tuple( env, "Failed to write into int dataset" ) ;
  }

  enif_free( buffer_for_hdf ) ;

  return atom_ok ;

}
