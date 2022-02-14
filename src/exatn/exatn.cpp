#include "exatn.hpp"

#ifdef MPI_ENABLED
#include "mpi.h"
#endif

#include <iostream>

namespace exatn {

#ifdef MPI_ENABLED
void initialize(const MPICommProxy & communicator,
                const ParamConf & parameters,
                const std::string & graph_executor_name,
                const std::string & node_executor_name)
{
  if(!exatnFrameworkInitialized){
    serviceRegistry->initialize();
    //std::cout << "#DEBUG(exatn): ExaTN services initialized" << std::endl << std::flush;
    exatnFrameworkInitialized = true;
    exatnInitializedMPI = false;
    numericalServer = std::make_shared<NumServer>(communicator,parameters,graph_executor_name,node_executor_name);
    bool synced = numericalServer->sync(); assert(synced);
    //std::cout << "#DEBUG(exatn): ExaTN numerical server initialized with MPI" << std::endl << std::flush;
  }
  return;
}
#endif


void initialize(const ParamConf & parameters,
                const std::string & graph_executor_name,
                const std::string & node_executor_name)
{
  if(!exatnFrameworkInitialized){
    serviceRegistry->initialize();
    //std::cout << "#DEBUG(exatn): ExaTN services initialized" << std::endl << std::flush;
    exatnFrameworkInitialized = true;
#ifdef MPI_ENABLED
    int thread_provided;
    int mpi_error = MPI_Init_thread(NULL,NULL,MPI_THREAD_MULTIPLE,&thread_provided);
    assert(mpi_error == MPI_SUCCESS);
    assert(thread_provided == MPI_THREAD_MULTIPLE);
    exatnInitializedMPI = true;
    numericalServer = std::make_shared<NumServer>(MPICommProxy(MPI_COMM_WORLD),parameters,
                                                  graph_executor_name,node_executor_name);
    bool synced = numericalServer->sync(); assert(synced);
    //std::cout << "#DEBUG(exatn): ExaTN numerical server initialized with MPI" << std::endl << std::flush;
#else
    exatnInitializedMPI = false;
    numericalServer = std::make_shared<NumServer>(parameters,graph_executor_name,node_executor_name);
    bool synced = numericalServer->sync(); assert(synced);
    //std::cout << "#DEBUG(exatn): ExaTN numerical server initialized" << std::endl << std::flush;
#endif
  }
  return;
}


bool isInitialized() {
  return exatnFrameworkInitialized;
}


void finalize() {
  if(exatnFrameworkInitialized){
    bool synced = numericalServer->sync(true,true); assert(synced);
    numericalServer.reset();
#ifdef MPI_ENABLED
    if(exatnInitializedMPI){
      int mpi_error = MPI_Finalize(); assert(mpi_error == MPI_SUCCESS);
      exatnInitializedMPI = false;
    }
#endif
    exatnFrameworkInitialized = false;
    //std::cout << "#DEBUG(exatn): ExaTN numerical server shut down" << std::endl << std::flush;
  }
  return;
}

} // namespace exatn
