cmake_minimum_required(VERSION 2.8)

# figure out if we use MinGW with Ninja: call with -DCC=gcc
if(WIN32 AND CMAKE_GENERATOR MATCHES Ninja AND CC MATCHES gcc)
	find_file(mingw_make mingw32-make.exe)
	get_filename_component(mingw_bin_dir ${mingw_make} PATH)
	set(CMAKE_RC_COMPILER ${mingw_bin_dir}/windres.exe)
	message("CMAKE_RC_COMPILER: ${CMAKE_RC_COMPILER}")
endif()

project(3rdparty)

include(ExternalProject)

set(build_dir ${CMAKE_BINARY_DIR}/3rdparty)
if(kst_3rdparty_dir)
	set(install_dir ${kst_3rdparty_dir})
else()
	set(install_dir ${build_dir}/installed)
endif()
set(generator -G${CMAKE_GENERATOR})
set(install_prefix -DCMAKE_INSTALL_PREFIX=${install_dir})
file(MAKE_DIRECTORY ${install_dir})

if(CMAKE_BUILD_TYPE)
	set(build_type -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()
message(STATUS "3rdparty build type: ${build_type}")
if(MINGW AND CMAKE_GENERATOR MATCHES Ninja)
	set(rc_compiler -DCMAKE_RC_COMPILER=${CMAKE_RC_COMPILER})
	set(install_target install\\strip)
else()
	set(install_target install/strip)
endif()

if(NOT kst_make_program)
	set(kst_make_program ${CMAKE_MAKE_PROGRAM})
endif()

# make dependencies visible for cmake
# http://public.kitware.com/Bug/view.php?id=13538
macro(kst_external_static_lib targetName name lib)
    add_library(${targetName} STATIC IMPORTED GLOBAL)
    set_target_properties(${targetName} PROPERTIES IMPORTED_LOCATION ${install_dir}/lib/${lib})
    add_dependencies(${targetName} ${name})
endmacro()



# GETDATA
set(ext_src ${build_dir}/getdata/src/getdata)
if(WIN32)
	set(ext_url http://sourceforge.net/projects/getdata/files/getdata/0.8.0/getdata_win-0.8.0.zip)
	set(ext_configure cmake ${ext_src}/cmake ${generator} ${rc_compiler} ${install_prefix} ${build_type})
	set(ext_install ${install_target})
	set(ext_make ${kst_make_program})
else()
	set(ver 0.7.3)
	set(ext_url http://sourceforge.net/projects/getdata/files/getdata/${ver}/getdata-${ver}.tar.gz)
	set(ext_configure ${ext_src}/configure --with-pic --enable-static --disable-shared --disable-fortran --disable-legacy-api --prefix=${install_dir})
	set(ext_install install)
	set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)
endif()
ExternalProject_Add(getdata
					PREFIX ${build_dir}/getdata
					URL ${ext_url}
					SOURCE_DIR ${ext_src}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE OFF
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} ${ext_install})

if(APPLE)
	set(gddeps -lbz2 -lz)
endif()
set(GETDATA_INCLUDEDIR ${install_dir}/include PARENT_SCOPE)
kst_external_static_lib(gdc_build   libgetdata libgetdata.a)
kst_external_static_lib(gdcpp_build libgetdata libgetdata++.a)
set(GETDATA_LIBRARIES gdcpp_build gdc_build ${gddeps} PARENT_SCOPE)


# ZLIB needed by MATIO
set(ext_src ${build_dir}/zlib/src/zlib)
if(WIN32)
	set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/zlib-develop-mingw-fixed.zip)
	set(ext_configure cmake ${ext_src} ${generator} -DBUILD_SHARED_LIBS=0 ${rc_compiler} ${install_prefix} ${build_type})
	set(ext_install ${install_target})
	set(ext_insource OFF )
		set(zlib_library libzlibstatic.a)
	set(ext_make ${kst_make_program})
else()
	set(ext_url http://zlib.net/zlib-1.2.7.tar.gz)
	set(ext_configure ${ext_src}/configure --static --prefix=${install_dir})
	set(ext_install install)
	set(ext_insource ON)
		set(zlib_library libz.a)
	set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)
endif()
ExternalProject_Add(zlib
					PREFIX ${build_dir}/zlib
					URL ${ext_url}
					SOURCE_DIR ${ext_src}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE ${ext_insource}
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} ${ext_install})

kst_external_static_lib(zlib_build zlib ${zlib_library})

# MATIO
set(ext_src ${build_dir}/matio/src/matio)
if(WIN32)
	set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/matio-2012.03.14.zip)
	set(ext_configure cmake ${CMAKE_CURRENT_SOURCE_DIR}/matio
											-Dmatio_dir=${ext_src}
											-DZLIB_INCLUDE_DIR=${install_dir}/include
											-DZLIB_LIBRARY=${zlib_library}
											${generator} ${rc_compiler}
											${install_prefix}
											${build_type})
	set(ext_install ${install_target})
	set(ext_insource OFF )
	set(ext_make ${kst_make_program})
else()
	set(ext_url http://sourceforge.net/projects/matio/files/matio/1.5.0/matio-1.5.0.tar.gz)
	set(ext_configure ${ext_src}/configure --with-pic --enable-static --disable-shared --prefix=${install_dir} --with-zlib=${install_dir})
	set(ext_install install)
	set(ext_insource ON)
	set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)
endif()
ExternalProject_Add(matio
					PREFIX ${build_dir}/matio
					URL ${ext_url}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE OFF
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} ${ext_install})
add_dependencies(matio zlib)

set(MATIO_INCLUDEDIR ${install_dir}/include PARENT_SCOPE)
kst_external_static_lib(matio_build matio libmatio.a)
set(MATIO_LIBRARIES matio_build zlib_build PARENT_SCOPE)


# NETCDF
set(ext_src ${build_dir}/netcdf/src/netcdf)
if(WIN32)
	set(ext_url ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.1.1.tar.gz)
	set(ext_configure cmake ${CMAKE_CURRENT_SOURCE_DIR}/netcdf
											-Dnetcdf_sources=${build_dir}/netcdf/src/netcdf
											${generator} ${rc_compiler}
											${install_prefix}
											${build_type})
	set(ext_install ${install_target})
	set(ext_insource OFF)
	set(ext_make ${kst_make_program})
else()
	set(ext_url ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.1.3.tar.gz)
	set(ext_configure ${ext_src}/configure  --with-pic  --disable-netcdf-4
											--enable-static --disable-shared   --disable-curl --disable-dap
											--prefix=${install_dir} --with-zlib=${install_dir})
	set(ext_install install)
	set(ext_insource ON)
	set(ext_make ${CMAKE_MAKE_PROGRAM})
endif()
ExternalProject_Add(netcdf
					PREFIX ${build_dir}/netcdf
					URL ${ext_url}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE OFF
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${ext_make} ${ext_install})

set(NETCDF_INCLUDEDIR  ${install_dir}/include PARENT_SCOPE)
kst_external_static_lib(netcdf_c_build netcdf libnetcdf.a)
kst_external_static_lib(netcdf_cpp_build netcdf libnetcdf_c++.a)
set(NETCDF_LIBRARY_C netcdf_c_build PARENT_SCOPE)
set(NETCDF_LIBRARY_CPP netcdf_cpp_build PARENT_SCOPE)

# CFITSIO
set(ext_src ${build_dir}/cfitsio/src/cfitsio)
if(WIN32)
	set(ext_url ftp://heasarc.gsfc.nasa.gov/software/fitsio/c/cfit3250.zip)
	set(ext_configure  cmake ${CMAKE_CURRENT_SOURCE_DIR}/cfitsio
											-DCFIT_DIR=${build_dir}/cfitsio/src/cfitsio
											${generator} ${rc_compiler}
											${install_prefix}
											${build_type})
	set(ext_install ${install_target})
	set(ext_insource OFF)
	set(ext_make ${kst_make_program})
else()
	set(ext_url ftp://heasarc.gsfc.nasa.gov/software/fitsio/c/cfitsio3300.tar.gz)
	set(ext_configure ${ext_src}/configure  --with-pic
											--enable-static --disable-shared
											--prefix=${install_dir} --with-zlib=${install_dir})
	set(ext_install install)
	set(ext_insource ON)
	set(ext_make ${CMAKE_MAKE_PROGRAM} CXXFLAGS=-fPIC CFLAGS=-fPIC)
endif()
ExternalProject_Add(cfitsio
					PREFIX ${build_dir}/cfitsio
					URL ${ext_url}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE ${ext_insource}
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} ${ext_install})


set(CFITSIO_INCLUDEDIR ${install_dir}/include/cfitsio PARENT_SCOPE)
kst_external_static_lib(cfitsio_build cfitsio libcfitsio.a)
set(CFITSIO_LIBRARIES cfitsio_build PARENT_SCOPE)



# GSL
set(ext_src ${build_dir}/gsl/src/gsl)
if(WIN32)
	set(ext_url http://sourceforge.net/projects/kst/files/3rdparty/gsl-1.14-patched.zip)
	set(ext_configure cmake ${CMAKE_CURRENT_SOURCE_DIR}/gsl
											-DGSL_DIR=${build_dir}/gsl/src/gsl
											${generator} ${rc_compiler}
											${install_prefix}
											${build_type})
	set(ext_install ${install_target})
	set(ext_insource OFF)
	set(ext_make ${kst_make_program})
else()
	set(ext_url ftp://ftp.gnu.org/gnu/gsl/gsl-1.15.tar.gz)
	set(ext_configure ${ext_src}/configure  --with-pic
											--enable-static --disable-shared
											--prefix=${install_dir} --with-zlib=${install_dir})
	set(ext_install install)
	set(ext_insource ON)
	set(ext_make ${CMAKE_MAKE_PROGRAM})
endif()
ExternalProject_Add(gsl
					PREFIX ${build_dir}/gsl
					URL ${ext_url}
					CONFIGURE_COMMAND ${ext_configure}
					BUILD_IN_SOURCE OFF
					BUILD_COMMAND ${ext_make}
					INSTALL_COMMAND ${ext_make} ${ext_install})

if(APPLE)
		set(GSL_DEPS ${install_dir}/lib/libgslcblas.a)
endif()
set(GSL_INCLUDEDIR ${install_dir}/include PARENT_SCOPE)
kst_external_static_lib(gsl_build gsl libgsl.a)
set(GSL_LIBRARIES gsl_build ${GSL_DEPS} PARENT_SCOPE)





