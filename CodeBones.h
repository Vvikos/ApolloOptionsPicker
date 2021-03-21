//===--- CodeBones.h ------------------------------------------------------===//
//
// APOLLO - Automatic speculative POLyhedral Loop Optimizer.
//
//===----------------------------------------------------------------------===//
//
// The BSD 3-Clause License
//
// Copyright (c) 2016. INRIA, CNRS and University of Strasbourg
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Main Contributors:
//     Professor Philippe Clauss     <clauss@unistra.fr>
//     Juan Manuel Martinez Caamano  <jmartinezcaamao@gmail.com>
//     Aravind Sukumaran-Rajam       <aravind_sr@outlook.com>
//     Artiom Baloian                <artiom.baloian@inria.fr>
//
// Participated as internships:
//     Willy Wolff
//     Matias Perez
//     Esteban Campostrini
//
//===----------------------------------------------------------------------===//

#ifndef BONES_H
#define BONES_H

#include "CodeBones/BonesInfo/BoneLoop.h"

#include <vector>
#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <string>
//Raquel new
#include "TxSelection/TxInfo.h"
//end Raquel new

// just the declaration..
namespace apollo {
class CodeGenerator;
} // end namespace apollo

struct APOLLO_PROG;

using osl_scop_t = struct osl_scop;
using CloogState = struct cloogstate;
using CloogOptions = struct cloogoptions;

struct clast_stmt;
class ArrayAnalysis;

struct ScopTypeStruct {
  std::shared_ptr<apollo::codebones::BoneLoop> Loop;
  osl_scop_t *Verification;
  osl_scop_t *Computation;
  bool SupportInspectorExecutor;

  ScopTypeStruct(std::shared_ptr<apollo::codebones::BoneLoop> &Loop_,
                 osl_scop_t *Verification_, osl_scop_t *Computation_);
  ~ScopTypeStruct();
  void dump(std::ostream &OS, std::string prefix);
};

struct ScanTypeStruct {
  std::shared_ptr<apollo::codebones::BoneLoop> Loop;
  CloogState *State;
  CloogOptions *Options;
  clast_stmt *ClastVerification;
  clast_stmt *ClastComputation;
  bool SupportInspectorExecutor;

  std::set<std::pair<std::string, std::set<int>>> Parallel[2];
  std::set<std::pair<std::string, std::set<int>>> Vectorize[2];
  std::map<std::string, std::pair<bool, int>> Unroll[2];
  std::map<int, std::string> StmtIdToName[2];

  ScanTypeStruct(int F = 99999);
  ~ScanTypeStruct();

  void dump(std::ostream &OS);
};

namespace apollo {

using ScopPtrTy = std::shared_ptr<ScopTypeStruct>;
using ScanPtrTy = std::shared_ptr<ScanTypeStruct>;

class CodeBoneManager {
  std::shared_ptr<codebones::BoneLoop> OutermostLoop;
  apollo::CodeGenerator &CodeGen;
  APOLLO_PROG *apolloProg;

  bool initCodeBonesNest(bool Computation, bool Verification);
  bool codeBonesNestValidDependencies(ArrayAnalysis &MemInfo);

public:
  CodeBoneManager(APOLLO_PROG *apolloProg_, apollo::CodeGenerator &CG);
  ScopPtrTy getScop();
  //ScopPtrTy getOptimized(ScopPtrTy &Original);
  //Raquel new
  ScopPtrTy getOptimized(ScopPtrTy &Original, TxInfo &InfoTx, std::string uniqueId);
  //end Raquel new
  ScanPtrTy getScan(ScopPtrTy &Scope);
};

} // end namespace apollo

#endif
