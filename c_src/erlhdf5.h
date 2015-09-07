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
 Forked on Thursday, August 6, 2015 by Olivier Boudeville
 (olivier.boudeville@esperide.com)
*/


#include "dbg.h"
#ifndef __erlhdf5_h__
#define __erlhdf5_h__

#include "hdf5.h"
#include "erl_nif.h"

#define MAXBUFLEN 1024


// Determines the number of elements of specified array:
#define NUM_OF(x) (sizeof(x) / sizeof *(x))


// Shared variables.

// Atoms (initialized in on_load):
ERL_NIF_TERM atom_ok ;
ERL_NIF_TERM atom_error ;

ErlNifResourceType* resource_type ;


// Resource type to pass pointers from C to Erlang:
typedef struct
{

  hid_t id ;

} Handle ;


// To identify the type of cell elements:
typedef enum { INTEGER, FLOAT } cell_type ;


// C prototypes for helpers:


ERL_NIF_TERM error_tuple( ErlNifEnv* env, char* reason ) ;

int convert_array_to_nif_array( ErlNifEnv* env, hsize_t size, hsize_t *arr_from,
  ERL_NIF_TERM* arr_to ) ;

int convert_nif_to_hsize_array( ErlNifEnv* env, hsize_t size,
  const ERL_NIF_TERM* arr_from, hsize_t *arr_to ) ;

int convert_int_array_to_nif_array( ErlNifEnv* env, hsize_t size, int *arr_from,
  ERL_NIF_TERM* arr_to ) ;


ERL_NIF_TERM write_float_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list,
  hid_t file_dataspace_id ) ;

ERL_NIF_TERM write_int_array( hid_t dataset_id, ErlNifEnv* env,
  unsigned int list_length, int tuple_size, ERL_NIF_TERM tuple_list,
  hid_t file_dataspace_id ) ;


// HDF5 C API:

// h5f sub-API;
ERL_NIF_TERM h5fcreate( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5fopen(   ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5fclose(  ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;


// h5s sub-API;
ERL_NIF_TERM h5screate_simple( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5sclose( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5sget_simple_extent_ndims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5sget_simple_extent_dims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5sselect_hyperslab( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;



// h5p sub-API;
ERL_NIF_TERM h5pcreate( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5pclose(  ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;


// h5t sub-API;
ERL_NIF_TERM h5tcopy(  ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5tclose( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5tget_class( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5tget_order( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5tget_size( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;


// h5d sub-API;
ERL_NIF_TERM h5dcreate( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5dopen(   ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;
ERL_NIF_TERM h5dclose(  ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5d_get_space_status( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5dwrite( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5d_get_storage_size( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5dget_type( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5dget_space(ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;


// h5lt sub-API;
ERL_NIF_TERM h5lt_make_dataset( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5lt_read_dataset_int( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5ltget_dataset_ndims( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;

ERL_NIF_TERM h5ltget_dataset_info( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] ) ;


#endif // __erlhdf5_h__
