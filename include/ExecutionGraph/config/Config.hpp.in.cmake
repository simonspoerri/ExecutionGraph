// ========================================================================================
//  ExecutionGraph 
//  Copyright (C) 2017 by Gabriel Nützi <gnuetzi (at) gmail (døt) com>//  
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================================

#ifndef ExecutionGraph_Config_Config_hpp_in_cmake
#define ExecutionGraph_Config_Config_hpp_in_cmake

//! Settings for the ExecutionGraph Library
namespace execGraph{

    //! Library Version
    static const unsigned int VersionMajor =  @ExecutionGraph_VERSION_MAJOR@ ;
    static const unsigned int VersionMinor =  @ExecutionGraph_VERSION_MINOR@ ;
    static const unsigned int VersionPatch =  @ExecutionGraph_VERSION_PATCH@ ;
    
    //! Force log level
    #define ExecutionGraph_FORCE_MSGLOG_LEVEL @ExecutionGraph_FORCE_MSGLOG_LEVEL@
    
    #cmakedefine ExecutionGraph_OPENMP_SUPPORT 
    #cmakedefine ExecutionGraph_OPENMP_USE_NTHREADS
    #define ExecutionGraph_OPENMP_NTHREADS @ExecutionGraph_OPENMP_NTHREADS@
    
    #ifdef ExecutionGraph_OPENMP_USE_NTHREADS
        #define ExecutionGraph_OPENMP_NUMTHREADS num_threads(ExecutionGraph_OPENMP_NTHREADS)
    #else
        #define ExecutionGraph_OPENMP_NUMTHREADS
    #endif
    
    #cmakedefine ExecutionGraph_THROW_IF_BAD_SOCKET_CASTS
    #ifdef ExecutionGraph_THROW_IF_BAD_SOCKET_CASTS
        #define EXECGRAPH_THROW_BADSOCKETCAST_IF(cond,mess) EXECGRAPH_THROW_EXCEPTION_TYPE_IF(cond,mess,BadSocketCastException)
    #else
        #define EXECGRAPH_THROW_BADSOCKETCAST_IF(cond,mess) EXECGRAPH_ASSERT_TYPE(cond,mess,BadSocketCastException)
    #endif
}


#endif
