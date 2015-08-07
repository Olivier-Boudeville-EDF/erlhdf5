%% Copyright (C) 2012 Roman Shestakov
%%%
%%% This file is part of erlhdf5
%%%
%%% erlhdf5 is free software: you can redistribute it and/or modify
%%% it under the terms of the GNU Lesser General Public License as
%%% published by the Free Software Foundation, either version 3 of
%%% the License, or (at your option) any later version.
%%%
%%% erlhdf5 is distributed in the hope that it will be useful,
%%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%%% GNU Lesser General Public License for more details.
%%%
%%% You should have received a copy of the GNU Lesser General Public
%%% License along with Erlsom.  If not, see
%%% <http://www.gnu.org/licenses/>.
%%%
%%% Author contact: romanshestakov@yahoo.co.uk

% Forked on Thursday, August 6, 2015 by Olivier Boudeville
% (olivier.boudeville@esperide.com)



-module(erlhdf5).

-export( [ h5fcreate/2, h5fopen/2, h5fclose/1 ] ).

-export( [ h5screate_simple/2, h5sclose/1, h5sget_simple_extent_dims/2,
		   h5sget_simple_extent_ndims/1 ] ).

-export( [ h5pcreate/1, h5pclose/1 ] ).

-export( [ h5tcopy/1, h5tclose/1, h5tget_class/1, h5tget_order/1, h5tget_size/1
		 ] ).

-export( [ h5dcreate/5, h5dopen/2, h5dclose/1, h5dget_type/1,
		   h5d_get_space_status/1, h5dwrite/2, h5d_get_storage_size/1,
		   h5dget_space/1 ] ).

-export( [ h5lt_make_dataset/5, h5lt_read_dataset_int/2,
		   h5ltget_dataset_ndims/2, h5ltget_dataset_info/3 ] ).


-include( "../include/erlhdf5.hrl" ).


-on_load( init/0 ).


-spec init() -> 'ok'.
init() ->

	% Typically loads erlhdf5.so (no extension must be used here):
	LibPath = filename:join( [ get_priv_dir( ?MODULE ), "erlhdf5" ] ),

	% io:format( "Initializing erlhdf5 library." ),

	ok = erlang:load_nif( LibPath, _LoadInfo=0 ).



% Returns a path to the 'priv' directory of specified module.
%
-spec get_priv_dir( atom() ) -> file:filename().
get_priv_dir( Module ) ->

	BasePath = filename:dirname( filename:dirname( code:which( Module ) ) ),

	filename:join( [ BasePath, "priv" ] ).



% Creates a (new) HDF5 file.
%
-spec h5fcreate( FileName::string(), Flag::atom() ) ->
					   { 'ok', integer() } | { 'error', atom() }.
h5fcreate( _FileName, _Flag ) ->
	nif_error( ?LINE ).


% Opens an (already-existing) HDF5 file.
%
-spec h5fopen( FileName::string(), Flag::atom() ) ->
				 { 'ok', FileHandler::integer() } | { 'error', Reason::atom() }.
h5fopen( _FileName, _Flag ) ->
	nif_error( ?LINE ).



% Closes specified HDF5 file.
%
-spec h5fclose( Handler::integer() ) -> 'ok' | { 'error', Reason::atom() }.
h5fclose( _Handler ) ->
	nif_error( ?LINE ).



% Creates (in-memory) a dataspace.
%
-spec h5screate_simple( Rank::integer(), Dimensions::tuple() ) ->
							  { 'ok', integer() } | { 'error', atom() }.
h5screate_simple( _Rank, _Dimensions ) ->
	nif_error( ?LINE ).



% Closes a dataspace.
%
-spec h5sclose( Handler::integer() ) -> 'ok' | { 'error', Reason::atom() }.
h5sclose( _Handler ) ->
	nif_error( ?LINE ).



% Returns the dimension size and maximum size of specified dataspace.
%
-spec h5sget_simple_extent_dims( Handler::integer(), Rank::integer() ) ->
		 { 'ok', Dims::list(), MaxDims::list() } | { 'error', Reason::atom() }.
h5sget_simple_extent_dims( _Handler, _Rank ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of a dataspace (list of current dimensions and of
% maximum ones).
%
-spec h5sget_simple_extent_ndims( Handler::integer() ) ->
				   { 'ok', NDIMS::integer() } | { 'error', Reason::atom() }.
h5sget_simple_extent_ndims( _Handler ) ->
	nif_error(?LINE).



% Creates a new property list as an instance of a property list class.
%
-spec h5pcreate( Class::string() ) -> { 'ok', integer() } | { 'error', atom() }.
h5pcreate( _Class ) ->
	nif_error( ?LINE ).



% Closes a property list.
%
-spec h5pclose( Handler::integer() ) -> 'ok' | { 'error', Reason::atom() }.
h5pclose( _Handler ) ->
	nif_error( ?LINE ).



% Copies an existing datatype and returns it.
%
-spec h5tcopy( Handler::integer() ) -> { 'ok', integer() }
										 | { 'error', Reason::atom() }.
h5tcopy( _Handler ) ->
	nif_error( ?LINE ).



% Closes a datatype.
%
-spec h5tclose( Handler::integer() ) -> 'ok' | { 'error', Reason::atom() }.
h5tclose( _Handler ) ->
	nif_error( ?LINE ).



% Returns the datatype class identifier.
%
-spec h5tget_class( Handler::integer() ) -> { 'ok', atom() }
												| { 'error', Reason::atom() }.
h5tget_class( _Handler ) ->
	nif_error( ?LINE ).



% Returns the byte order of an atomic datatype.
%
-spec h5tget_order( Handler::integer() ) -> { 'ok', integer() }
												| { 'error', Reason::atom() }.
h5tget_order( _Handler ) ->
	nif_error( ?LINE ).



% Returns the size of a datatype.
%
-spec h5tget_size( Handler::integer() ) -> { 'ok', integer() }
											   | { 'error', Reason::atom() }.
h5tget_size( _Handler ) ->
	nif_error( ?LINE ).



% Creates a new dataset.
%
-spec h5dcreate( File::integer(), Name::string(), Type::integer(),
				 Space::integer(), Prop::integer() ) ->
					   { 'ok', binary() } | { 'error', atom() }.
h5dcreate( _File, _Name, _Type, _Space, _Prop ) ->
	nif_error( ?LINE ).



% Opens an existing dataset.
%
-spec h5dopen( File::integer(), Name::string() ) ->
					 { 'ok', binary() } | { 'error', atom() }.
h5dopen( _File, _Name ) ->
	nif_error( ?LINE ).


% Closes specified dataset.
%
-spec h5dclose( Handler::integer() ) -> 'ok' | { 'error', Reason::atom() }.
h5dclose( _Handler ) ->
	nif_error( ?LINE ).



% Returns an identifier for a copy of the datatype for a dataset.
%
-spec h5dget_type( Handler::integer() ) -> { 'ok', integer() }
											   | { 'error', Reason::atom() }.
h5dget_type( _Handler ) ->
	nif_error( ?LINE ).



% Determines whether space has been allocated for the specified dataset.
%
-spec h5d_get_space_status( Handler::integer() ) ->
					{ 'ok', Status::atom() } | { 'error', Reason::atom() }.
h5d_get_space_status( _Handler ) ->
	nif_error( ?LINE ).



-spec h5dwrite( DSetHandler::binary(), Data::binary() ) ->
					  'ok' | { 'error', Reason::atom() }.
h5dwrite( _DSetHandler, _Data ) ->
	nif_error( ?LINE ).



% Returns the amount of storage allocated for a dataset.
%
-spec h5d_get_storage_size( Handler::integer() ) ->
				  { 'ok', Size::integer() } | { 'error', Reason::atom() }.
h5d_get_storage_size( _Handler ) ->
	nif_error( ?LINE ).



% Returns an identifier for a copy of the dataspace for a dataset.
%
-spec h5dget_space( Handler::integer() ) ->
				  { 'ok', Size::integer() } | { 'error', Reason::atom() }.
h5dget_space( _Handler ) ->
	nif_error( ?LINE ).



% Writes a dataset to file.
%
-spec h5lt_make_dataset( FileHandler::integer(), DatasetName::string(),
						 Rank::integer(), Dims::list(), Data::list()) ->
							   'ok' | { 'error', Reason::atom() }.
h5lt_make_dataset( _FileHandler, _DatasetName, _Rank, _Dims, _Data ) ->
	nif_error( ?LINE ).



% Reads a dataset from file.
%
-spec h5lt_read_dataset_int( Handler::integer(), DatasetName::string() ) ->
								   'ok' | { 'error', Reason::atom() }.
h5lt_read_dataset_int( _Handler, _DatasetName ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of a dataset.
%
-spec h5ltget_dataset_ndims( Handler::integer(), DatasetName::string() ) ->
				   { 'ok', integer() } | { 'error', Reason::atom() }.
h5ltget_dataset_ndims( _Handler, _DatasetName ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of specified dataset.
%
-spec h5ltget_dataset_info( Handler::integer(), DatasetName::string(),
							Rank::integer() ) ->
						  { 'ok', list() } | { 'error', Reason::atom() }.
h5ltget_dataset_info( _Handler, _DatasetName, _Rank ) ->
	nif_error( ?LINE ).



% Helper, called whenever a NIF-related error occurs (typically if not having
% loaded the relevant code).
%
nif_error( Line ) ->
	exit( { nif_library_not_loaded, { module, ?MODULE },
			{ line, Line } } ).
