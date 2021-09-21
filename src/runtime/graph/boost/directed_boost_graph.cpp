/** ExaTN:: Tensor Runtime: Directed acyclic graph of tensor operations
REVISION: 2021/09/21

Copyright (C) 2018-2021 Dmitry Lyakh, Alex McCaskey, Tiffany Mintz
Copyright (C) 2018-2021 Oak Ridge National Laboratory (UT-Battelle)
**/

#include "directed_boost_graph.hpp"

#include <iostream>

using namespace boost;

namespace exatn {
namespace runtime {

DirectedBoostGraph::DirectedBoostGraph():
 dag_(std::make_shared<d_adj_list>())
{
}


VertexIdType DirectedBoostGraph::addOperation(std::shared_ptr<TensorOperation> op) {
  lock();
  auto vid = add_vertex(*dag_);
  (*dag_)[vid].properties = std::move(std::make_shared<TensorOpNode>(op));
  (*dag_)[vid].properties->setId(vid); //DAG node id is stored in the node properties
  auto output_tensor = op->getTensorOperand(0); //output tensor operand
  bool dependent = false; int epoch;
  const auto * nodes = exec_state_.getTensorEpochNodes(*output_tensor,&epoch);
  if(nodes != nullptr){
    for(const auto & node_id: *nodes) addDependency(vid,node_id); //Write-after-Read & Write-after-Write
    dependent = true;
  }
  exec_state_.registerTensorWrite(*output_tensor,vid);
  unsigned int num_operands = op->getNumOperands();
  for(unsigned int i = 1; i < num_operands; ++i){ //input tensor operands
    auto tensor = op->getTensorOperand(i);
    nodes = exec_state_.getTensorEpochNodes(*tensor,&epoch);
    if(epoch < 0){ //write epoch: Read-after-Write
      for(const auto & node_id: *nodes) addDependency(vid,node_id);
      dependent = true;
    }
    exec_state_.registerTensorRead(*tensor,vid);
  }
  //if(!dependent) exec_state_.registerDependencyFreeNode(vid);
  unlock();
  return vid; //new node id in the DAG
}


void DirectedBoostGraph::addDependency(VertexIdType dependent, VertexIdType dependee) {
  lock();
  add_edge(vertex(dependent,*dag_), vertex(dependee,*dag_), *dag_);
  unlock();
  return;
}


bool DirectedBoostGraph::dependencyExists(VertexIdType vertex_id1, VertexIdType vertex_id2) {
  lock();
  auto vid1 = vertex(vertex_id1, *dag_);
  auto vid2 = vertex(vertex_id2, *dag_);
  auto p = edge(vid1, vid2, *dag_);
  unlock();
  return p.second;
}


TensorOpNode & DirectedBoostGraph::getNodeProperties(VertexIdType vertex_id) {
  lock();
  TensorOpNode & node_properties = *((*dag_)[vertex_id].properties);
  unlock();
  return node_properties;
}


std::size_t DirectedBoostGraph::getNodeDegree(VertexIdType vertex_id) {
  lock();
//return boost::degree(vertex(vertex_id, *dag_), *dag_);
  std::size_t ns = getNeighborList(vertex_id).size();
  unlock();
  return ns;
}


std::size_t DirectedBoostGraph::getNumNodes() {
  lock();
  std::size_t n = num_vertices(*dag_);
  unlock();
  return n;
}


std::size_t DirectedBoostGraph::getNumDependencies() {
  lock();
  std::size_t m = num_edges(*dag_);
  unlock();
  return m;
}


std::vector<VertexIdType> DirectedBoostGraph::getNeighborList(VertexIdType vertex_id) {
  std::vector<VertexIdType> l;

  lock();

  typedef typename boost::property_map<d_adj_list, boost::vertex_index_t>::type IndexMap;
  IndexMap indexMap = get(boost::vertex_index, *dag_);

  typedef typename boost::graph_traits<d_adj_list>::adjacency_iterator adjacency_iterator;

  std::pair<adjacency_iterator, adjacency_iterator> neighbors =
    boost::adjacent_vertices(vertex(vertex_id, *dag_), *dag_);

  for (; neighbors.first != neighbors.second; ++neighbors.first) {
    VertexIdType neighborIdx = indexMap[*neighbors.first];
    l.push_back(neighborIdx);
  }

  unlock();

  return l;
}


void DirectedBoostGraph::computeShortestPath(VertexIdType startIndex,
                                             std::vector<double> & distances,
                                             std::vector<VertexIdType> & paths) {
  lock();

  typename property_map<d_adj_list, edge_weight_t>::type weightmap =
           get(edge_weight, *dag_);
  std::vector<VertexIdType> p(num_vertices(*dag_));
  std::vector<std::size_t> d(num_vertices(*dag_));
  d_vertex_type s = vertex(startIndex, *dag_);

  dijkstra_shortest_paths(
      *dag_, s,
      predecessor_map(boost::make_iterator_property_map(
                             p.begin(), get(boost::vertex_index, *dag_)))
          .distance_map(boost::make_iterator_property_map(
                               d.begin(), get(boost::vertex_index, *dag_))));

  for (const auto & di: d) distances.push_back(static_cast<double>(di));
  for (const auto & pi: p) paths.push_back(pi);

  unlock();

  return;
}


void DirectedBoostGraph::printIt()
{
  lock();
  std::cout << "#MSG: Printing DAG:" << std::endl;
  auto num_nodes = num_vertices(*dag_);
  for(VertexIdType i = 0; i < num_nodes; ++i){
    auto deps = getNeighborList(i);
    std::cout << "Node " << i << ": Depends on { ";
    for(const auto & node_id: deps) std::cout << node_id << " ";
    std::cout << "}" << std::endl;
  }
  std::cout << "#END MSG" << std::endl;
  unlock();
  return;
}


void DirectedBoostGraph::clear()
{
  lock();
  dag_->clear();
  exec_state_.clear();
  unlock();
  return;
}

} // namespace runtime
} // namespace exatn
