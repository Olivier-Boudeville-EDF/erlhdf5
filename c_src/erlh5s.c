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

#include <stdio.h>
#include <stdlib.h>

#include "hdf5.h"

#include "erl_nif.h"

#include "dbg.h"
#include "erlhdf5.h"


// H5S: Dataspace Interface, dataspace definition and access routines.


/*
 * Creates a new simple dataspace, and opens it for access, returning a
 * dataspace identifier.
 *
 * -spec h5screate_simple( rank(), tuple( dimension_size() ) ) ->
 *   { 'ok', dataspace_handle() } | error().
 *
 */
ERL_NIF_TERM h5screate_simple( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  // Parses arguments:
  check( argc == 2, "Incorrect number of arguments" ) ;

  // Number of dimensions of dataspace:
  int rank ;
  check( enif_get_int( env, argv[0], &rank ), "Cannot get rank from argv" ) ;

  int arity ;
  const ERL_NIF_TERM* terms ;

  check( enif_get_tuple( env, argv[1], &arity, &terms ),
	"Cannot get dimension sizes from argv" ) ;

  // Makes sure that rank is matching arity:
  check( rank <= 2, "Up to two dimensions supported only" ) ;

  // Allocates array of size rank, specifying the size of each dimension:
  hsize_t * dimsf = (hsize_t*) enif_alloc( arity * sizeof( hsize_t ) ) ;

  // Copies the specified dimensions into dimsf:
  check( ! convert_nif_to_hsize_array( env, arity, terms, dimsf ),
	"Cannot convert dimensions array" ) ;

  // Creates a new dataspace, using default properties:
  hid_t dataspace_id = H5Screate_simple( rank, dimsf, NULL ) ;
  check( dataspace_id > 0, "Failed to create dataspace." ) ;

  // Clean-up:
  enif_free( dimsf ) ;
  ERL_NIF_TERM ret = enif_make_int( env, dataspace_id ) ;
  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if ( dataspace_id )
	H5Sclose( dataspace_id ) ;

  if ( dimsf )
	enif_free( dimsf ) ;

  return error_tuple( env, "Cannot create dataspace" ) ;

}


// Closes specified dataspace.
ERL_NIF_TERM h5sclose( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  check( argc == 1, "Incorrect number of arguments" ) ;

  hid_t dataspace_id ;

  check( enif_get_int( env, argv[0], &dataspace_id ),
	"Cannot get dataspace handle from argv" ) ;

  check( ! H5Sclose( dataspace_id ), "Failed to close dataspace" ) ;

  return atom_ok ;

 error:
  return error_tuple( env, "Cannot close dataspace" ) ;

}



// Determines the dimensionality of specified dataspace.
ERL_NIF_TERM h5sget_simple_extent_ndims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 1, "Incorrect number of arguments" ) ;

  hid_t dataspace_id ;
  check( enif_get_int( env, argv[0], &dataspace_id ),
	"Cannot get dataspace handle from argv" ) ;

  int ndims = H5Sget_simple_extent_ndims( dataspace_id ) ;

  check( ndims > 0, "Failed to determine dataspace dimensions." ) ;

  return enif_make_tuple2( env, atom_ok, enif_make_int( env, ndims ) ) ;

 error:
  return error_tuple( env, "Failed to determine dataspace dimensions" ) ;

}



// Selects an hyperslab in specified dataspace.
ERL_NIF_TERM h5sselect_hyperslab( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{


  /*
   * Note: memory leaks on unlikely error cases (ex: stride of different size
   * than offset).
   *
   */

  /* Type specification is:

  -spec h5sselect_hyperslab( dataspace_handle(), selection_operator(),
							 Offset::size_tuple(), Stride::size_tuple(),
							 Count::size_tuple(), Block::size_tuple() ) ->
								 'ok' | error().
  */

  if ( argc != 6 )
	return error_tuple( env, "Expected 6 arguments" ) ;


  hid_t dataspace_id ;

  if ( ! enif_get_int( env, argv[0], &dataspace_id ) )
	return error_tuple( env, "Cannot get dataspace handle from argv" ) ;

  char selection_operator[ MAXBUFLEN ] ;

  if ( ! enif_get_atom( env, argv[1], selection_operator, MAXBUFLEN,
	  ERL_NIF_LATIN1 ) )
	return error_tuple( env, "Cannot get selection operator from argv" ) ;

  H5S_seloper_t selection_op ;

  if ( strncmp( selection_operator, "H5S_SELECT_SET", MAXBUFLEN ) == 0 )
  {

	//printf( "Selection operator: 'set'.\n" ) ;
	selection_op = H5S_SELECT_SET ;

  }
  else if ( strncmp( selection_operator, "H5S_SELECT_OR", MAXBUFLEN ) == 0 )
  {

	//printf( "Selection operator: 'or'.\n" ) ;
	selection_op = H5S_SELECT_OR ;

  }
  else
  {

	char message[ MAXBUFLEN ] ;

	sprintf( message, "Unknown selection operator %s", selection_operator ) ;

	return error_tuple( env, message ) ;

  }


  /*
   * Let's now extract the four tuples about the dimensions of the hyperslab; we
   * expect each of them to have the same size and contain (positive) integers.
   *
   * We have to extract all their coordinates and store them in a C array so
   * that libhdf5 can be fed.
   *
   */

  const ERL_NIF_TERM * tuple_elements ;
  int tuple_size ;

  // Offset is argc=2:
  if ( ! enif_get_tuple( env, argv[2], &tuple_size, &tuple_elements ) )
  {

	/*
	 * Here Offset is not a tuple, hence we must be in a monodimensional
	 * setting, which is special-cased here:
	 *
	 */

	//tuple_size = 1 ;

	int int_offset ;
	if ( ! enif_get_int( env, argv[2], &int_offset ) )
	  return error_tuple( env, "Offset does not contain an integer" ) ;
	hsize_t offset = int_offset ;

	int int_stride ;
	if ( ! enif_get_int( env, argv[3], &int_stride ) )
	  return error_tuple( env, "Stride does not contain an integer" ) ;
	hsize_t stride = int_stride ;

	int int_count ;
	if ( ! enif_get_int( env, argv[4], &int_count ) )
	  return error_tuple( env, "Count does not contain an integer" ) ;
	hsize_t count = int_count ;

	int int_block ;
	if ( ! enif_get_int( env, argv[5], &int_block ) )
	  return error_tuple( env, "Block does not contain an integer" ) ;
	hsize_t block = int_block ;

	//printf( "offset = %llu, stride = %llu, count = %llu, block = %llu.\n",
	//  offset, stride, count, block ) ;

	if ( H5Sselect_hyperslab( dataspace_id, selection_op, &offset, &stride,
		&count, &block ) < 0  )
	{

	  //H5Eprint( H5Eget_current_stack(), /* error stream */ stderr ) ;

	  return error_tuple( env, "Hyperslab selection failed" ) ;

	}

	//printf( "Selection succeeded.\n" ) ;

	return atom_ok ;

  }

  // Here we are at least bidimensional.

  //printf( "Offset of size %u.\n", tuple_size ) ;

  // Kept for comparison:
  int overall_tuple_size = tuple_size ;


  int buffer_size = tuple_size * sizeof( hsize_t ) ;

  hsize_t * offset_buffer = enif_alloc( buffer_size ) ;

  unsigned int i ;

  int cell ;

  for ( i = 0 ; i < tuple_size ; i++ )
  {

	  if ( ! enif_get_int( env, tuple_elements[i], &cell ) )
	  {
		enif_free( offset_buffer ) ;
		return error_tuple( env, "Offset cell does not contain an integer" ) ;

	  }

	  offset_buffer[i] = (hsize_t) cell ;

  }


  // Stride is argc=3:
  if ( ! enif_get_tuple( env, argv[3], &tuple_size, &tuple_elements ) )
	  return error_tuple( env, "Stride parameter is not a tuple" ) ;

  //printf( "Stride of size %u.\n", tuple_size ) ;

  if ( overall_tuple_size != tuple_size )
	  return error_tuple( env, "Offset and stride of different sizes" ) ;

  hsize_t * stride_buffer = enif_alloc( buffer_size ) ;

  for ( i = 0 ; i < tuple_size ; i++ )
  {

	  if ( ! enif_get_int( env, tuple_elements[i], &cell ) )
	  {

		enif_free( offset_buffer ) ;
		enif_free( stride_buffer ) ;
		return error_tuple( env, "Stride cell does not contain an integer" ) ;

	  }

	  stride_buffer[i] = (hsize_t) cell ;

  }

  // Count is argc=4:
  if ( ! enif_get_tuple( env, argv[4], &tuple_size, &tuple_elements ) )
	  return error_tuple( env, "Count parameter is not a tuple" ) ;

  //printf( "Count of size %u.\n", tuple_size ) ;

  if ( overall_tuple_size != tuple_size )
	  return error_tuple( env, "Offset and count of different sizes" ) ;

  hsize_t * count_buffer = enif_alloc( buffer_size ) ;

  for ( i = 0 ; i < tuple_size ; i++ )
  {

	  if ( ! enif_get_int( env, tuple_elements[i], &cell ) )
	  {

		enif_free( offset_buffer ) ;
		enif_free( stride_buffer ) ;
		enif_free( count_buffer ) ;
		return error_tuple( env, "Count cell does not contain an integer" ) ;

	  }

	  count_buffer[i] = (hsize_t) cell ;

  }


  // Block is argc=5:
  if ( ! enif_get_tuple( env, argv[5], &tuple_size, &tuple_elements ) )
	  return error_tuple( env, "Block parameter is not a tuple" ) ;

  //printf( "Block of size %u.\n", tuple_size ) ;

  if ( overall_tuple_size != tuple_size )
	  return error_tuple( env, "Offset and block of different sizes" ) ;

  hsize_t * block_buffer = enif_alloc( buffer_size ) ;

  for ( i = 0 ; i < tuple_size ; i++ )
  {

	  if ( ! enif_get_int( env, tuple_elements[i], &cell ) )
	  {

		enif_free( offset_buffer ) ;
		enif_free( stride_buffer ) ;
		enif_free( count_buffer ) ;
		enif_free( block_buffer ) ;
		return error_tuple( env, "Block cell does not contain an integer" ) ;

	  }

	  block_buffer[i] = (hsize_t) cell ;

  }

  // Now that we have our four buffers right:

  if ( H5Sselect_hyperslab( dataspace_id, selection_op, offset_buffer,
	  stride_buffer, count_buffer, block_buffer ) < 0  )
  {

	//H5Eprint( H5Eget_current_stack(), /* error stream */ stderr ) ;

	enif_free( offset_buffer ) ;
	enif_free( stride_buffer ) ;
	enif_free( count_buffer ) ;
	enif_free( block_buffer ) ;
	return error_tuple( env, "Hyperslab selection failed" ) ;

  }

  //printf( "Selection succeeded.\n" ) ;

  return atom_ok ;

}



// Retrieves dataspace dimension sizes: current and maximum sizes.
ERL_NIF_TERM h5sget_simple_extent_dims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 2, "Incorrect number of arguments" ) ;

  hid_t dataspace_id ;
  check( enif_get_int( env, argv[0], &dataspace_id ),
	"Cannot get dataspace handle from argv" ) ;

  int rank ;
  check( enif_get_int( env, argv[1], &rank ), "Cannot get rank from argv" ) ;

  /*
   * Allocates space for dims array to store a number of dimensions equal to
   * rank:
   */
  int array_size = rank * sizeof( hsize_t ) ;

  hsize_t *dims = NULL ;
  dims = enif_alloc( array_size ) ;

  hsize_t *maxdims = NULL;
  maxdims = enif_alloc( array_size ) ;

  // Gets these dimensions from dataspace:
  int status = H5Sget_simple_extent_dims( dataspace_id, dims, maxdims ) ;
  check( status > 0, "Failed to get dimensions" ) ;


  // Allocates memory for arrays of ERL_NIF_TERM for the upcoming conversion:

  int nif_array_size = rank * sizeof( ERL_NIF_TERM ) ;

  ERL_NIF_TERM* dims_arr =    (ERL_NIF_TERM*) enif_alloc( nif_array_size ) ;
  ERL_NIF_TERM* maxdims_arr = (ERL_NIF_TERM*) enif_alloc( nif_array_size ) ;

  // Convert arrays into array of ERL_NIF_TERM:

  check( ! convert_array_to_nif_array( env, rank, dims, dims_arr ),
	"Cannot convert current array" ) ;

  check( ! convert_array_to_nif_array( env, rank, maxdims, maxdims_arr ),
	"Cannot convert max array" ) ;

   // Convert arrays to lists:

  ERL_NIF_TERM dims_list = enif_make_list_from_array( env, dims_arr, rank ) ;

  ERL_NIF_TERM maxdims_list = enif_make_list_from_array( env, maxdims_arr,
	rank ) ;

   // Cleanup:
  enif_free( dims ) ;
  enif_free( maxdims ) ;

  return enif_make_tuple3( env, atom_ok, dims_list, maxdims_list ) ;

 error:
  if ( dims )
	enif_free( dims ) ;

  if ( maxdims )
	enif_free( maxdims ) ;

  return error_tuple(env, "Cannot get dimensions" ) ;

}
