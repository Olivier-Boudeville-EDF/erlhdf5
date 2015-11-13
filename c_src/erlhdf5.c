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


#include <stdio.h>
#include <stdlib.h>

#include "erl_nif.h"


#include "erlhdf5.h"


// Main HDF5 binding file.


// Called because of enif_open_resource_type/5:
void resource_destructor( ErlNifEnv* env, void* obj )
{

}



void info( char * message )
{

  fprintf( stderr, "[INFO] %s", message ) ;

}


void display_error( char * message )
{

  char * error =  ( errno == 0 ? "None" : strerror( errno ) ) ;

  fprintf( stderr, "[ERROR] %s (errno: %s) ", message, error ) ;

}



// Loads this NIF.
static int load( ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info )
{

  info( "Loading erlhdf5 NIF." ) ;

  const char* module_name = "erlhdf5" ;
  const char* resource_name = "FileHandle" ;

  int resource_flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER ;

  ErlNifResourceFlags* tried = NULL ;

  resource_type = enif_open_resource_type( env, module_name,
	resource_name, /* resource_destructor */ NULL, resource_flags, tried ) ;

  if ( ! resource_type )
  {

	display_error( "Unable to open resource type." ) ;

	return -1 ;

  }

  // Initializes common atoms:
  atom_ok    = enif_make_atom( env, "ok" ) ;
  atom_error = enif_make_atom( env, "error" ) ;

  return 0 ;

}



/*
 * Converts specified error message from C to:
 *   { error::atom(), Reason::string() }
 *
 */
ERL_NIF_TERM error_tuple( ErlNifEnv* env, char* reason )
{

	ERL_NIF_TERM nif_reason = enif_make_string( env, reason, ERL_NIF_LATIN1 ) ;

	// { error, Reason }, thus:
	return enif_make_tuple2( env, atom_error, nif_reason ) ;

}


/* // convert array on ints to array of ErlNifEnv */
/* int convert_int_arr_to_nif_array(ErlNifEnv* env, int arity, int* arr_from, ErlNifEnv* arr_to) */
/* { */
/*   /\* arr_to = (ErlNifEnv*) enif_alloc(arity * sizeof(ErlNifEnv)); *\/ */
/*   int i; */
/*   for(i = 0; i < arity; i++) { */
/*     check(enif_get_int(env, arr_from[i], arr_to[i]), "error converting int to ErlNifEnv"); */
/*   } */
/*   return 0; */

/*  error: */
/*   return -1; */
/* }; */



/*
 * API name, arity and C name of all functions offered by the NIF:
 *
 * (note that apparently if a function is declared and listed yet not defined in
 * C, then the module will be loadable, yet even calls to functions that were
 * defined will fail as if they were not, ex: {undef,[{erlhdf5,h5fcreate,...)
 *
 */

static ErlNifFunc nif_funcs[] =
{

  { "h5fcreate",                  2, h5fcreate },
  { "h5fopen",                    2, h5fopen },
  { "h5fclose",                   1, h5fclose },

  { "h5screate_simple",           2, h5screate_simple },
  { "h5sclose",                   1, h5sclose },
  { "h5sget_simple_extent_dims",  2, h5sget_simple_extent_dims },
  { "h5sget_simple_extent_ndims", 1, h5sget_simple_extent_ndims },
  { "h5sselect_hyperslab",        6, h5sselect_hyperslab },

  { "h5pcreate",                  1, h5pcreate },
  { "h5pclose",                   1, h5pclose },

  { "datatype_name_to_handle",    1, datatype_name_to_handle },
  { "h5tcopy",                    1, h5tcopy },
  { "h5tclose",                   1, h5tclose },
  { "h5tget_class",               1, h5tget_class },
  { "h5tget_order",               1, h5tget_order },
  { "h5tget_size",                1, h5tget_size },

  { "h5dcreate",                  5, h5dcreate },
  { "h5dopen",                    2, h5dopen },
  { "h5dopen",                    3, h5dopen },
  { "h5dclose",                   1, h5dclose },
  { "h5dget_type",                1, h5dget_type },
  { "h5d_get_space_status",       1, h5d_get_space_status },
  { "h5dwrite",                   2, h5dwrite },
  { "h5dwrite",                   3, h5dwrite },
  { "h5d_get_storage_size",       1, h5d_get_storage_size },
  { "h5dget_space",               1, h5dget_space },

  { "h5lt_make_dataset",          5, h5lt_make_dataset },
  { "h5lt_read_dataset_int",      2, h5lt_read_dataset_int },
  { "h5lt_read_dataset_double",   2, h5lt_read_dataset_double },
  { "h5lt_read_dataset_string",   2, h5lt_read_dataset_string },
  { "h5ltget_dataset_ndims",      2, h5ltget_dataset_ndims },
  { "h5ltget_dataset_info",       3, h5ltget_dataset_info }

} ;


// Module name, NIF array, four callbacks (load, reload, upgrade, unload):
ERL_NIF_INIT( erlhdf5, nif_funcs, &load, NULL, NULL, NULL ) ;
