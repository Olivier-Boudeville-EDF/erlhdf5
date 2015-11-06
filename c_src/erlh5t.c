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



// H5T: Datatype Interface, datatype creation and manipulation routines.


/*
 * Converts an HDF5 type, expressed as a string, into the actual corresponding
 * HDF5 type (as an integer).
 *
 */
static int convert_type( const char* string_type, hid_t* target_hdf_type )
{

  // The full, longer list of HDF5 types is defined in ./include/H5Tpublic.h.

  if( strncmp( string_type, "H5T_NATIVE_INT", MAXBUFLEN ) == 0 )
	*target_hdf_type = H5T_NATIVE_INT;

  else if( strncmp( string_type, "H5T_NATIVE_LONG", MAXBUFLEN ) == 0 )
	*target_hdf_type = H5T_NATIVE_LONG;

  else if( strncmp( string_type, "H5T_NATIVE_FLOAT", MAXBUFLEN ) == 0 )
	*target_hdf_type = H5T_NATIVE_FLOAT;

  else if( strncmp( string_type, "H5T_NATIVE_DOUBLE", MAXBUFLEN ) == 0 )
	*target_hdf_type = H5T_NATIVE_DOUBLE;

  else
	sentinel( "Type '%s' not currently supported by erlhdf5", string_type ) ;

  return 0 ;

 error:
  return -1 ;

}



/*
 * Converts a data type, specified as an atom, to its handle (integer) HDF5
 * representation (without making a copy of it).
 *
 * -spec datatype_name_to_handle( datatype_name() ) ->
 *     { 'ok', datatype_handle() } | error().
 *
 */
ERL_NIF_TERM datatype_name_to_handle( ErlNifEnv* env, int argc,
  const ERL_NIF_TERM argv[] )
{

  check( argc == 1, "Incorrect number of arguments" ) ;

  char type[ MAXBUFLEN ] ;
  check( enif_get_atom( env, argv[0], type, sizeof( type ), ERL_NIF_LATIN1 ),
	"Cannot get type from argv" ) ;

  // Converts type (string -> handle) to a format that the HDF5 API understands:

  hid_t dtype_id ;
  check( ! convert_type( type, &dtype_id ), "Failed to convert datatype" ) ;


  ERL_NIF_TERM ret = enif_make_int( env, dtype_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:

  return error_tuple( env, "Cannot translate datatype" ) ;


}


/*
 * Returns a copy of the specified datatype (as an atom, like 'H5T_NATIVE_INT')
 * and returns it (as an handle).
 *
 * -spec h5tcopy( datatype_name() ) -> { 'ok', datatype_handle() } | error().
 *
 */
ERL_NIF_TERM h5tcopy( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  check( argc == 1, "Incorrect number of arguments" ) ;

  char type[ MAXBUFLEN ] ;
  check( enif_get_atom( env, argv[0], type, sizeof( type ), ERL_NIF_LATIN1 ),
	"Cannot get type from argv" ) ;

  // Converts type (string -> handle) to a format that the HDF5 API understands:

  hid_t dtype_id ;
  check( ! convert_type( type, &dtype_id ), "Failed to convert datatype" ) ;

  hid_t type_id = H5Tcopy( dtype_id ) ;
  check( type_id > 0, "Failed to create datatype." ) ;

  ERL_NIF_TERM ret = enif_make_int( env, type_id ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  if ( type_id )
	H5Tclose( type_id ) ;

  return error_tuple( env, "Cannot copy datatype" ) ;

}


// close
ERL_NIF_TERM h5tclose(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  hid_t type_id;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &type_id), "cannot get resource from argv");

  // close properties list
  check(!H5Tclose(type_id), "Failed to close type.");
  return atom_ok;

 error:
  return error_tuple(env, "cannot close type");
};


// Returns the datatype class identifier.
ERL_NIF_TERM h5tget_class(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ERL_NIF_TERM ret;
  H5T_class_t class_id;
  hid_t type_id;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &type_id), "cannot get resource from argv");

  class_id = H5Tget_class(type_id);
  //fprintf(stderr, "class type: %d\r\n", class_id);
  check(class_id != H5T_NO_CLASS, "Failed to get type class.");

  ret = enif_make_int(env, class_id);
  return enif_make_tuple2(env, atom_ok, ret);

 error:
  return error_tuple(env, "cannot get type class");
};


// Returns the byte order of an atomic datatype.
ERL_NIF_TERM h5tget_order(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
  ERL_NIF_TERM ret;
  H5T_order_t order;
  hid_t type_id;

  // parse arguments
  check(argc == 1, "Incorrect number of arguments");
  check(enif_get_int(env, argv[0], &type_id), "cannot get resource from argv");

  order = H5Tget_order(type_id);
  check(order != H5T_ORDER_ERROR, "Failed to get order.");

  ret = enif_make_int(env, order);
  return enif_make_tuple2(env, atom_ok, ret);

 error:
  return error_tuple(env, "Cannot get order") ;

}


// Returns the size of specified datatype, in bytes.
ERL_NIF_TERM h5tget_size( ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[] )
{

  check( argc == 1, "Incorrect number of arguments" ) ;

  hid_t type_id ;

  check( enif_get_int( env, argv[0], &type_id ),
	"Cannot get resource from argv" ) ;

  size_t size = H5Tget_size( type_id ) ;
  check( size > 0, "Failed to get datatype size." ) ;

  ERL_NIF_TERM ret = enif_make_int( env, size ) ;

  return enif_make_tuple2( env, atom_ok, ret ) ;

 error:
  return error_tuple( env, "Cannot get datatype size" ) ;

}
