//===--- RuntimeConfig.h --------------------------------------------------===//
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

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include "ProfilingPlugin/ProfilingPluginBase.h"
#include <JitCompiler/CodeGenerator.h>

#include <string>
#include <vector>
#include <map>
#include <memory>

class RuntimeConfig {
public:

  /**
   * The number of threads to use din optimized chunk
   */
  int NumberOfThreads;

  /**
   * Inside profiling chunks which size is specified by
   * InstrumentationChunkSize (see below), this specifies
   * the number of iterations to be profiled, for each
   * level of the loop nest.
   */
  int InstrumentationSample = InstrumentationChunkSize;

  /**
   * The number of iterations of the outermost loop
   * to be instrumented before building a prediction
   * model
   */
  int InstrumentationChunkSize = 16;

  /**
   * The number of iterations of the outermost loop
   * to be executed in an optimized fashion once a
   * prediction model has been built
   */
  int OptimizedChunkSize = 1024;

  /**
   * The mode of execution of APOLLO.
   * normal mode or profiling mode
   */
  bool profilingMode;

  /**
   * Wether or not APOLLO predicts all memory accesses,
   * scalars and bounds even if some cannot be predicted
   * as affine functions or tubes.
   */
  bool predictAll;

  /**
   * The list of registered profiling plugins
   * when running in profiling mode
   */
  std::vector<ProfilingPluginBase*> ProfilingPlugins;

  /**
   * In profiling mode, this frequency indicates
   * at which frequency instrumentation chunks
   * (which size is InstrumentationChunkSize defined
   * above) are executed.
   *
   * A frequency of -1 means to run all the outer most
   * loop as instrumentation chunks. In other words, to
   * instrument all the execution.
   */
  int ProfilingFrequency = -1;

  //Raquel new
  std::unordered_map<std::string, int> itersProcessed;

  std::unordered_map<std::string, std::unordered_set<std::string>> allPlutoOptsTried;

  /**
   * This number specifies how many transformations must be tried
   * before choosing the best one
   */
  int TxsToTry;

  /**
   * Table of the selected transformation for each nested loop analyzed,
   * as well as a list of the transformations already tried
   */
  //std::unordered_map<std::string, std::pair<apollo::TxInfo, std::unordered_set<std::string>>> TxTableLoop;

  std::unordered_map<std::string, std::vector<apollo::TxInfo>> TxTableLoop;

  //std::vector<apollo::TxInfo> TxsPerLoop;

  /**
   * Set of pluto options to try for each loop nest
   * 
   */
  std::vector<std::pair<PlutoOptions*, std::vector<int>>> PlutoOptsSet;

  /**
   * If true, execute APOLLO with the multiversioning mechanism
   * If false, execute standard APOLLO
   */
  bool multiVersioning;
  bool multiThreading;
  bool launchOriginal;
  int seqThreadBehavior;

  struct Ehrhart{
    int d;
    int unknowns;
    double maxIters;
    double *C_ij;
    bool compStart;
    bool compEnd;
    bool dynamic;
    bool notLinear;
    bool solved;
  };
  std::unordered_map<int64_t, Ehrhart> loopDepths;
  std::unordered_map<int64_t, float> origTime;

  int NumberOfDataflowThreads;

  struct EMode{
    bool launchSeqThread = true;
    bool abortApollo = true;
    bool timeout = false;
    double timeoutValue = -1;
    double maxTime = -1;
  };

  EMode ExecutionMode;

  bool apolloAsSlave;

  std::string dynamicParams;

  struct Actor{
    std::string actorName;
    bool stopTxSelection = false;
    bool useApollo = false;
    bool active = false;
    bool isDynamic = false;
    int numberOfLoops = 0;
  };

  std::unordered_map<unsigned long, std::vector<Actor>> threadActor;

  //end Raquel new
  RuntimeConfig();
  void storeApolloProg(int64_t loopId);
  void storePM();
  void retrievePM();
  void storePhiState(apollo::PhiState &PhiStateObj);
  RuntimeConfig::Ehrhart computeNumberOfUnknowns(int64_t currentLoopId, int d, bool restart);
  /*int registerDataflowThread();
  int removeDataflowThread();*/
  void setMode(int mode);
  void printApolloMode();
  void setApolloAsSlave();
  void createPlutoOptsSet();
//Raquel new
  ~RuntimeConfig();
//end Raquel new
private:
  void setIntFromEnv(const char *Name, int &Var);
  void setBoolFromEnv(const char *Name, bool &Var);
  void setStringFromEnv(const char *Name, std::string &Var);
  void setDoubleFromEnv(const char *Name, double &Var);
};

#endif
