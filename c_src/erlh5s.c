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


// Creates a new simple dataspace, and opens it for access.
ERL_NIF_TERM h5screate_simple( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  const ERL_NIF_TERM *terms ;
  int rank; // number of dimensions of dataspace
  int arity;

  // Parses arguments:
  check( argc == 2, "Incorrect number of arguments" ) ;
  check( enif_get_int( env, argv[0], &rank ), "Cannot get rank from argv" ) ;
  check( enif_get_tuple( env, argv[1], &arity, &terms ),
	"Cannot get terms from argv");

  // Makes sure that rank is matching arity:
  check( rank <= 2, "does not support more than 2 dimensions" ) ;

  // Allocates array of size rank, specifiying the size of each dimension:
  hsize_t * dimsf = (hsize_t*) enif_alloc( arity * sizeof( hsize_t ) ) ;

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


// close
ERL_NIF_TERM h5sclose(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  hid_t dataspace_id;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &dataspace_id), "cannot get resource from argv");
  // close properties list
  check(!H5Sclose(dataspace_id), "Failed to close dataspace.");
  return atom_ok;

 error:
  return error_tuple(env, "cannot close dataspace");
};


// Determines the dimensionality of a dataspace.
ERL_NIF_TERM h5sget_simple_extent_ndims(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  hid_t dataspace_id;
  int ndims;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &dataspace_id), "cannot get resource from argv");

  // get ndims
  ndims = H5Sget_simple_extent_ndims(dataspace_id);
  check(ndims > 0, "Failed to determine dataspace dimensions.");
  return enif_make_tuple2(env, atom_ok, enif_make_int(env, ndims));

 error:
  return error_tuple(env, "Failed to determine dataspace dimensions");
};


// Retrieves dataspace dimension size and maximum size.
ERL_NIF_TERM h5sget_simple_extent_dims(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  hid_t dataspace_id;
  hsize_t *dims = NULL;
  hsize_t *maxdims = NULL;
  int status;
  ERL_NIF_TERM dims_list;
  ERL_NIF_TERM maxdims_list;
  ERL_NIF_TERM* dims_arr;
  ERL_NIF_TERM* maxdims_arr;
  int rank;

  // parse arguments
  check(argc == 2, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &dataspace_id), "cannot get resource from argv");
  check(enif_get_int(env, argv[1], &rank), "cannot get rank from argv");

  // allocate space for dims array to store a number of dimensions
  dims = enif_alloc(rank * sizeof(hsize_t));
  maxdims = enif_alloc(rank * sizeof(hsize_t));

  // get a number of dims from dataspace
  status = H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
  check(status > 0, "Failed to get dims.");

  // allocate mem for arrays of ERL_NIF_TERM so we could convert
  dims_arr = (ERL_NIF_TERM*)enif_alloc(sizeof(ERL_NIF_TERM) * rank);
  maxdims_arr = (ERL_NIF_TERM*)enif_alloc(sizeof(ERL_NIF_TERM) * rank);

  // convert arrays into array of ERL_NIF_TERM
  check(!convert_array_to_nif_array(env, rank, dims, dims_arr), "cannot convert array");
  check(!convert_array_to_nif_array(env, rank, maxdims, maxdims_arr), "cannot convert array");

   // convert arrays to list
  dims_list = enif_make_list_from_array(env, dims_arr, rank);
  maxdims_list = enif_make_list_from_array(env, maxdims_arr, rank);

   // cleanup
  enif_free(dims);
  enif_free(maxdims);
  return enif_make_tuple3(env, atom_ok, dims_list, maxdims_list);

 error:
  if(dims) enif_free(dims);
  if(maxdims) enif_free(maxdims);
  return error_tuple(env, "cannot get dims");
};
