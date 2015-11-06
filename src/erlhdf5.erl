% Copyright (C) 2012 Roman hestakov
%
% This file is part of erlhdf5
%
% erlhdf5 is free software: you can redistribute it and/or modify it under the
% terms of the GNU Lesser General Public License as published by the Free
% Software Foundation, either version 3 of the License, or (at your option) any
% later version.
%
% erlhdf5 is distributed in the hope that it will be useful, but WITHOUT ANY
% WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
% A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
% details.
%
% You should have received a copy of the GNU Lesser General Public License along
% with erlhdf5. If not, see <http://www.gnu.org/licenses/>.
%
% Author contact: romanshestakov@yahoo.co.uk

% Forked on Thursday, August 6, 2015 by Olivier Boudeville
% (olivier.boudeville@esperide.com)



-module(erlhdf5).


% H5F, about HDF5 files:
-export( [ h5fcreate/2, h5fopen/2, h5fclose/1 ] ).


% H5S, about dataspaces:
-export( [ h5screate_simple/2, h5sclose/1, h5sget_simple_extent_dims/2,
		   h5sget_simple_extent_ndims/1, h5sselect_hyperslab/6 ] ).


% H5P, about property lists:
-export( [ h5pcreate/1, h5pclose/1 ] ).


% H5T, about datatypes:
-export( [ datatype_name_to_handle/1, h5tcopy/1, h5tclose/1, h5tget_class/1,
		   h5tget_order/1, h5tget_size/1
		 ] ).


% H5D, about datasets:
-export( [ h5dcreate/5, h5dopen/2, h5dopen/3, h5dclose/1, h5dget_type/1,
		   h5d_get_space_status/1, h5dwrite/2, h5dwrite/3,
		   h5d_get_storage_size/1, h5dget_space/1 ] ).


% H5LT, about HDF5 Lite:
-export( [ h5lt_make_dataset/5, h5lt_read_dataset_int/2,
		   h5lt_read_dataset_double/2, h5ltget_dataset_ndims/2,
		   h5ltget_dataset_info/3 ] ).



-include( "../include/erlhdf5.hrl" ).


% Number of elements (ex: in a dimension):
-type size() :: integer().


% A tuple whose elements are of type size(), corresponding to as many
% dimensions:
%
-type size_tuple() :: tuple(). % tuple( size() )




% Either replaces the existing dataspace selection or adds a new one.
%
%-type selection_operator() :: 'set' | 'or'.
-type selection_operator() :: 'H5S_SELECT_SET' | 'H5S_SELECT_OR'.


-type handle() :: integer().

-type file_handle()          :: handle().
-type dataset_handle()       :: handle().
-type dataspace_handle()     :: handle().

-type datatype_name()        :: atom().
-type datatype_handle()      :: handle().

-type property_list_handle() :: handle().

-type dataset_creation_proplist() :: property_list_handle().
-type dataset_access_proplist()   :: property_list_handle().


-type error() :: { 'error', Reason::string() }.

-type rank() :: integer().
-type dimensions() :: tuple().


% Element of an array (a dataset):
-type cell_element() :: 'native_int' | 'native_float'.


% Data typically read or written.
-type data() :: list(). % More precisely: list( tuple( cell_element() ) ).


-export_type([
			   size/0, size_tuple/0, selection_operator/0,
			   file_handle/0, dataset_handle/0, dataspace_handle/0,
			   datatype_handle/0, property_list_handle/0,
			   dataset_creation_proplist/0, dataset_access_proplist/0,
			   error/0, rank/0, dimensions/0,
			   cell_element/0, data/0
			 ]).


-on_load( init/0 ).



% Following sections will be listed in turn:
%
% - overall: generalities
% - H5F: about HDF5 files
% - H5S: about dataspaces
% - H5P: about property lists
% - H5T: about datatypes
% - H5D: about dataset
% - H5LT: about HDF5 Lite
% - helpers



% Overall base section.


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





% H5F section: about HDF5 files.



% Creates a (new) HDF5 file.
%
-spec h5fcreate( FileName::string(), Flag::atom() ) ->
					   { 'ok', file_handle() } | error().
h5fcreate( _FileName, _Flag ) ->
	nif_error( ?LINE ).



% Opens an (already-existing) HDF5 file.
%
-spec h5fopen( FileName::string(), Flag::atom() ) ->
				 { 'ok', file_handle() } | error().
h5fopen( _FileName, _Flag ) ->
	nif_error( ?LINE ).



% Closes specified HDF5 file.
%
-spec h5fclose( file_handle() ) -> 'ok' | error().
h5fclose( _Handle ) ->
	nif_error( ?LINE ).





% H5S section: about dataspaces.



% Creates a dataspace.
%
-spec h5screate_simple( rank(), dimensions() ) ->
							  { 'ok', dataspace_handle() } | error().
h5screate_simple( _Rank, _Dimensions ) ->
	nif_error( ?LINE ).



% Closes a dataspace.
%
-spec h5sclose( dataspace_handle() ) -> 'ok' | error().
h5sclose( _Handle ) ->
	nif_error( ?LINE ).



% Returns the dimension size and maximum size of specified dataspace.
%
-spec h5sget_simple_extent_dims( dataspace_handle(), rank() ) ->
		 { 'ok', Dims::list(), MaxDims::list() } | error().
h5sget_simple_extent_dims( _Handle, _Rank ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of a dataspace (list of current dimensions and of
% maximum ones).
%
-spec h5sget_simple_extent_ndims( dataspace_handle() ) ->
				   { 'ok', NDIMS::integer() } | error().
h5sget_simple_extent_ndims( _Handle ) ->
	nif_error( ?LINE ).



% Selects an hyperslab in specified dataspace.
%
% See https://www.hdfgroup.org/HDF5/doc/H5.user/Dataspaces.html for more
% information.
%
-spec h5sselect_hyperslab( dataspace_handle(), selection_operator(),
						   Offset::size_tuple(), Stride::size_tuple(),
						   Count::size_tuple(), Block::size_tuple() ) ->
								 'ok' | error().
h5sselect_hyperslab( _DataspaceHandle, _SelectionOperator, _Offset, _Stride,
					 _Count, _Block ) ->
	nif_error( ?LINE ).




% H5P section: about property lists.



% Creates a new property list as an instance of a property list class.
%
-spec h5pcreate( Class::string() ) ->
					   { 'ok', property_list_handle() } | error().
h5pcreate( _Class ) ->
	nif_error( ?LINE ).



% Closes a property list.
%
-spec h5pclose( property_list_handle() ) -> 'ok' | error().
h5pclose( _Handle ) ->
	nif_error( ?LINE ).




% H5T section: about datatypes.


% Converts a data type, specified as an atom, to its handle (integer) HDF5
% representation (without making a copy of it).
%
% (binding exported helper)
%
-spec datatype_name_to_handle( datatype_name() ) ->
									 { 'ok', datatype_handle() } | error().
datatype_name_to_handle( _DatatypeName ) ->
	nif_error( ?LINE ).


% Copies an existing datatype (specified as an atom like 'H5T_NATIVE_INT') and
% returns it (as an handle).
%
-spec h5tcopy( datatype_name() ) -> { 'ok', datatype_handle() } | error().
h5tcopy( _DatatypeName ) ->
	nif_error( ?LINE ).



% Closes a datatype.
%
-spec h5tclose( datatype_handle() ) -> 'ok' | error().
h5tclose( _Handle ) ->
	nif_error( ?LINE ).



% Returns the datatype class identifier.
%
-spec h5tget_class( datatype_handle() ) -> { 'ok', atom() } | error().
h5tget_class( _Handle ) ->
	nif_error( ?LINE ).



% Returns the byte order of an atomic datatype.
%
-spec h5tget_order( datatype_handle() ) -> { 'ok', integer() } | error().
h5tget_order( _Handle ) ->
	nif_error( ?LINE ).



% Returns the size of a datatype.
%
-spec h5tget_size( datatype_handle() ) -> { 'ok', integer() } | error().
h5tget_size( _Handle ) ->
	nif_error( ?LINE ).




% H5D section: about dataset.



% Creates a new dataset.
%
-spec h5dcreate( File::file_handle(), Name::string(), Type::datatype_handle(),
				 Space::dataspace_handle(),
				 Prop::dataset_creation_proplist() ) ->
					   { 'ok', dataset_handle() } | error().
h5dcreate( _File, _Name, _Type, _Space, _Prop ) ->
	nif_error( ?LINE ).



% Opens an existing dataset.
%
-spec h5dopen( file_handle(), Name::string() ) ->
					 { 'ok', dataset_handle() } | error().
h5dopen( _File, _Name ) ->
	nif_error( ?LINE ).



% Opens an existing dataset with specified access property list.
%
-spec h5dopen( File::file_handle(), Name::string(), AccessPropList::any() ) ->
					 { 'ok', dataset_handle() } | error().
h5dopen( _File, _Name, _AccessPropList ) ->
	nif_error( ?LINE ).



% Closes specified dataset.
%
-spec h5dclose( dataset_handle() ) -> 'ok' | error().
h5dclose( _Dataset ) ->
	nif_error( ?LINE ).



% Returns an identifier for a copy of the datatype for a dataset.
%
-spec h5dget_type( dataset_handle() ) -> { 'ok', datatype_handle() } | error().
h5dget_type( _Dataset ) ->
	nif_error( ?LINE ).



% Determines whether space has been allocated for the specified dataset.
%
-spec h5d_get_space_status( dataset_handle() ) ->
					{ 'ok', Status::atom() } | error().
h5d_get_space_status( _Dataset ) ->
	nif_error( ?LINE ).



% Writes specified data into specified dataset.
%
-spec h5dwrite( dataset_handle(), data() ) -> 'ok' | error().
h5dwrite( _Dataset, _Data ) ->
	nif_error( ?LINE ).



% Writes specified data into specified dataset, using specified dataspace.
%
% Useful for partial file writing.
%
-spec h5dwrite( dataset_handle(), dataspace_handle(), data() ) ->
					  'ok' | error().
h5dwrite( _Dataset, _FileDataspace, _Data ) ->
	nif_error( ?LINE ).



% Returns the amount of storage allocated for a dataset.
%
-spec h5d_get_storage_size( dataset_handle() ) ->
								  { 'ok', Size::integer() } | error().
h5d_get_storage_size( _Handle ) ->
	nif_error( ?LINE ).



% Returns an identifier for a copy of the dataspace for a dataset.
%
-spec h5dget_space( dataset_handle() ) ->
						  { 'ok', dataspace_handle() } | error().
h5dget_space( _Handle ) ->
	nif_error( ?LINE ).




% H5LT section: about HDF5 Lite.


% Writes a dataset to file.
%
-spec h5lt_make_dataset( file_handle(), DatasetName::string(), rank(),
						 Dims::dimensions(), Data::list() ) -> 'ok' | error().
h5lt_make_dataset( _FileHandler, _DatasetName, _Rank, _Dims, _Data ) ->
	nif_error( ?LINE ).



% Reads specified integer dataset from specified file.
%
-spec h5lt_read_dataset_int( file_handle(), DatasetName::string() ) ->
								   { 'ok', [ integer() ] } | error().
h5lt_read_dataset_int( _Handle, _DatasetName ) ->
	nif_error( ?LINE ).


% Reads specified double dataset from specified file.
%
-spec h5lt_read_dataset_double( file_handle(), DatasetName::string() ) ->
								   { 'ok', [ float() ] } | error().
h5lt_read_dataset_double( _Handle, _DatasetName ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of a dataset.
%
-spec h5ltget_dataset_ndims( dataset_handle(), DatasetName::string() ) ->
				   { 'ok', integer() } | error().
h5ltget_dataset_ndims( _Handle, _DatasetName ) ->
	nif_error( ?LINE ).



% Returns the dimensionality of specified dataset.
%
-spec h5ltget_dataset_info( dataset_handle(), DatasetName::string(), rank() ) ->
								  { 'ok', list() } | error().
h5ltget_dataset_info( _Handle, _DatasetName, _Rank ) ->
	nif_error( ?LINE ).




% Helper section.


% Helper, called whenever a NIF-related error occurs (typically if not having
% loaded the relevant code).
%
nif_error( Line ) ->
	exit( { nif_library_not_loaded, { module, ?MODULE }, { line, Line } } ).
