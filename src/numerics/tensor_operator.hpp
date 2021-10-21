/** ExaTN::Numerics: Tensor operator
REVISION: 2021/10/21

Copyright (C) 2018-2021 Dmitry I. Lyakh (Liakh)
Copyright (C) 2018-2021 Oak Ridge National Laboratory (UT-Battelle) **/

/** Rationale:
 (a) A tensor network vector is a vector in a given tensor space with
     its expansion (tensor) coefficients factorized as a tensor network.
     A ket tensor network vector produces its corresponding dual bra
     tensor network vector upon complex conjugation of all constituting
     tensor factors and reversing the direction of all tensor legs.
 (b) A tensor network operator is an ordered linear combination of tensors
     and tensor networks in which the output tensor legs are distinguished
     as bra and ket tensor legs: The bra tensor legs contract with legs
     of a bra tensor network vector, the ket tensor legs contract with
     legs of a ket tensor network vector.
 (c) Different components of a tensor network operator do not have to have
     the same number of ket or bra legs, but the corresponding tensor space
     where the tensor network operator is used must have sufficiently large
     rank to accomodate the action of the ket and bra legs.
 (d) The first component of the tensor network operator is applied first
     when acting on a ket vector; the last component of the tensor network
     operator is applied first when acting on a bra vector.
 (e) The order of components of a tensor network operator is reversed upon conjugation.
**/

#ifndef EXATN_NUMERICS_TENSOR_OPERATOR_HPP_
#define EXATN_NUMERICS_TENSOR_OPERATOR_HPP_

#include "tensor_basic.hpp"
#include "tensor_network.hpp"

#include <string>
#include <vector>
#include <complex>
#include <memory>

#include "errors.hpp"

namespace exatn{

namespace numerics{

class TensorOperator{
public:

 //Tensor operator component:
 struct OperatorComponent{
  //Tensor network (or a single tensor stored as a tensor network of size 1):
  std::shared_ptr<TensorNetwork> network;
  //Ket legs of the tensor network: Global tensor mode id <-- Output tensor leg:
  std::vector<std::pair<unsigned int, unsigned int>> ket_legs;
  //Bra legs of the tensor network: Global tensor mode id <-- Output tensor leg:
  std::vector<std::pair<unsigned int, unsigned int>> bra_legs;
  //Expansion coefficient of the operator component:
  std::complex<double> coefficient;
 };

 using Iterator = typename std::vector<OperatorComponent>::iterator;
 using ConstIterator = typename std::vector<OperatorComponent>::const_iterator;

 TensorOperator(const std::string & name): name_(name) {}

 TensorOperator(const TensorOperator &) = default;
 TensorOperator & operator=(const TensorOperator &) = default;
 TensorOperator(TensorOperator &&) noexcept = default;
 TensorOperator & operator=(TensorOperator &&) noexcept = default;
 virtual ~TensorOperator() = default;

 inline Iterator begin() {return components_.begin();}
 inline Iterator end() {return components_.end();}
 inline ConstIterator cbegin() const {return components_.cbegin();}
 inline ConstIterator cend() const {return components_.cend();}

 inline OperatorComponent & operator[](std::size_t component_num){
  assert(component_num < components_.size());
  return components_[component_num];
 }

 /** Returns the name of the tensor network operator. **/
 inline const std::string & getName() const{
  return name_;
 }

 /** Returns the total number of components in the tensor operator. **/
 inline std::size_t getNumComponents() const{
  return components_.size();
 }

 /** Returns a specific component of the tensor operator. **/
 inline const OperatorComponent & getComponent(std::size_t component_num){
  assert(component_num < components_.size());
  return components_[component_num];
 }

 /** Appends a new component to the tensor operator linear expansion. The new component
     can either be a tensor network or just a single tensor expressed as a tensor network
     of size 1. The ket and bra pairing arguments specify which legs of the network output
     tensor act on a ket vector and which on a bra vector, together with their mapping onto
     the global modes of the tensor space the tensor operator is supposed to act upon. **/
 bool appendComponent(std::shared_ptr<TensorNetwork> network,                                 //in: tensor network (or single tensor as a tensor network)
                      const std::vector<std::pair<unsigned int, unsigned int>> & ket_pairing, //in: ket pairing: Global tensor mode id <-- Output tensor leg
                      const std::vector<std::pair<unsigned int, unsigned int>> & bra_pairing, //in: bra pairing: Global tensor mode id <-- Output tensor leg
                      const std::complex<double> coefficient);                                //in: expansion coefficient

 /** Appends a new component to the tensor operator linear expansion. The new component is
     a single tensor. The ket and bra pairing arguments specify which legs of the tensor
     act on a ket vector and which on a bra vector, together with their mapping onto
     the global modes of the tensor space the tensor operator is supposed to act upon. **/
 bool appendComponent(std::shared_ptr<Tensor> tensor,                                         //in: tensor
                      const std::vector<std::pair<unsigned int, unsigned int>> & ket_pairing, //in: ket pairing: Global tensor mode id <-- Tensor leg
                      const std::vector<std::pair<unsigned int, unsigned int>> & bra_pairing, //in: bra pairing: Global tensor mode id <-- Tensor leg
                      const std::complex<double> coefficient);                                //in: expansion coefficient

 /** Appends the given tensor network into the tensor network operator multiple times
     by assigning the ket and bra legs of the output tensor of the appended network
     to the global tensor space modes in all possible uniquely permuted fully-ordered ways. **/
 bool appendSymmetrizeComponent(std::shared_ptr<TensorNetwork> network,        //in: tensor network (or single tensor as a tensor network)
                                const std::vector<unsigned int> & ket_pairing, //in: ket tensor legs (output tensor)
                                const std::vector<unsigned int> & bra_pairing, //in: bra tensor legs (output tensor)
                                unsigned int ket_space_rank,                   //in: rank of the global ket tensor space
                                unsigned int bra_space_rank,                   //in: rank of the global bra tensor space
                                const std::complex<double> coefficient,        //in: expansion coefficient
                                bool antisymmetrize = false);                  //in: whether or not to negate the coefficient for odd permutations

 /** Appends the given tensor into the tensor network operator multiple times
     by assigning the ket and bra legs of the tensor to the global tensor space
     modes in all possible uniquely permuted fully-ordered ways. **/
 bool appendSymmetrizeComponent(std::shared_ptr<Tensor> tensor,                //in: tensor
                                const std::vector<unsigned int> & ket_pairing, //in: ket tensor legs
                                const std::vector<unsigned int> & bra_pairing, //in: bra tensor legs
                                unsigned int ket_space_rank,                   //in: rank of the global ket tensor space
                                unsigned int bra_space_rank,                   //in: rank of the global bra tensor space
                                const std::complex<double> coefficient,        //in: expansion coefficient
                                bool antisymmetrize = false);                  //in: whether or not to negate the coefficient for odd permutations

 /** Deletes the specified component of the tensor operator. **/
 bool deleteComponent(std::size_t component_num);

 /** Conjugates the tensor operator: All constituting tensors are complex conjugated,
     all tensor legs reverse their direction, bra legs and ket legs are swapped,
     complex linear expansion coefficients are complex conjugated. **/
 void conjugate();

 /** Returns linear combination coefficients for all components. **/
 std::vector<std::complex<double>> getCoefficients() const;

 /** Prints. **/
 void printIt() const;

protected:

 std::string name_;                          //tensor operator name
 std::vector<OperatorComponent> components_; //ordered components of the tensor operator
};

} //namespace numerics

/** Creates a new tensor network operator as a shared pointer. **/
template<typename... Args>
inline std::shared_ptr<numerics::TensorOperator> makeSharedTensorOperator(Args&&... args)
{
 return std::make_shared<numerics::TensorOperator>(std::forward<Args>(args)...);
}

} //namespace exatn

#endif //EXATN_NUMERICS_TENSOR_OPERATOR_HPP_
