//===--- RuntimeConfig.cpp ------------------------------------------------===//
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

#include <ProfilingPlugin/LinearPhasesDetector.h>
#include "RuntimeConfig.h"
#include "Backdoor/Backdoor.h"
#include "ProfilingPlugin/ProfilingPluginBase.h"
#include <cstdlib>
#include <iostream>
#include <omp.h>
#include <ProfilingPlugin/ReIndexPhasesDetector.h>

// Options Picker
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

/// \brief Sets an integer value from env (if found)
/// \param Name
/// \param Var
void RuntimeConfig::setIntFromEnv(const char *Name, int &Var) {
	const char *Env = getenv(Name);
	if (Env)
		Var = atoi(Env);
}

/// \brief Sets an bool value from env (if found)
///        The value is set as true if the env variable is not zero
/// \param Name
/// \param Var
void RuntimeConfig::setBoolFromEnv(const char *Name, bool &Var) {
	const char *Env = getenv(Name);
	if (Env)
		Var = atoi(Env) != 0;
}

/// \brief Sets an string value from env (if found)
/// \param Name
/// \param Var
void RuntimeConfig::setStringFromEnv(const char *Name, std::string &Var) {
	const char *Env = getenv(Name);
	if (Env)
		Var = Env;
}
//Raquel new
/// \brief Sets a double value from env (if found)
/// \param Name
/// \param Var
void RuntimeConfig::setDoubleFromEnv(const char *Name, double &Var) {
	const char *Env = getenv(Name);
	if (Env)
		Var = std::stod(Env);
}

static PlutoOptions *generatePlutoOptions(bool parallel, bool tile, bool intratileopt, 
	bool l2tile, bool identity, bool rar, bool partlbtile, bool lbtile, int fuse, bool unroll, int unrollfactor, bool useIsl){
	identity =
      backdoorutils::getEnvBool("APOLLO_BONES", "--identity") ? true : identity;
    tile =
      backdoorutils::getEnvBool("APOLLO_BONES", "--notile")
          ? false
          : (backdoorutils::getEnvBool("APOLLO_BONES", "--tile") ? true : tile);
    useIsl =
      backdoorutils::getEnvBool("APOLLO_BONES", "--islsolve") ? true : useIsl;
    parallel = 
      backdoorutils::getEnvBool("APOLLO_BONES", "--noparallel") ? false : parallel;
    intratileopt = 
      backdoorutils::getEnvBool("APOLLO_BONES", "--nointratile") ? false : intratileopt;
	PlutoOptions *Options = pluto_options_alloc();
	Options->quiet = true;
	Options->debug = false;
	Options->moredebug = false;
	Options->silent = true;
	Options->prevector = tile;
	Options->tile = tile;
	Options->intratileopt = intratileopt;
	Options->l2tile = l2tile; 
	Options->parallel = parallel;
	Options->fuse = fuse;
	Options->unroll = unroll;
	Options->ufactor = unrollfactor;
	Options->iss = true; 
	Options->innerpar = false;
	Options->candldep = !useIsl;
	Options->isldep = useIsl;
	Options->islsolve = useIsl;
	Options->identity = identity;
	Options->lbtile = lbtile;
	Options->partlbtile = partlbtile;
	Options->rar = rar;
	Options->forceparallel = false;
	Options->multipipe = false;
	Options->polyunroll = false;
	Options->time = false;

	return Options;
}
//end Raquel new

// constructor
RuntimeConfig::RuntimeConfig() {
	//Raquel new
	printf("THIS IS apolloRuntime WITH MULTIVERSIONING\n");
	//itersProcessed = 0;
	TxsToTry = 9;
	multiThreading=false;
	multiVersioning=false;
	apolloAsSlave = false;
	launchOriginal = false;
	NumberOfDataflowThreads = 0;
	multiVersioning = backdoorutils::getEnvBool("APOLLO_MULTIVERSIONING", "");
	multiThreading = backdoorutils::getEnvBool("APOLLO_MULTITHREADING", "");
	
	//printf("Multiversioning? %d\n", multiVersioning);
	//multiVersioning=1;
	seqThreadBehavior = 0;
	setIntFromEnv("APOLLO_SEQTHREAD", seqThreadBehavior);
	setIntFromEnv("APOLLO_TXTRY", TxsToTry);
	//printf("Sequential thread behavior? %d\n", seqThreadBehavior);
	setMode(seqThreadBehavior);

	if(multiVersioning){
		createPlutoOptsSet();
	}
	
	//end Raquel new
	setIntFromEnv("APOLLO_INSTR_CHUNK_SIZE", InstrumentationChunkSize);
	setIntFromEnv("APOLLO_OPT_CHUNK_SIZE", OptimizedChunkSize);
	InstrumentationSample = InstrumentationChunkSize;
	setIntFromEnv("APOLLO_INSTR_SAMPLE", InstrumentationSample);
	NumberOfThreads = omp_get_max_threads();

	predictAll = backdoorutils::getEnvBool("APOLLO_PREDICT_ALL", "");

	// Profiling mode
	setIntFromEnv("APOLLO_PROFILING_FREQUENCY", ProfilingFrequency);
	profilingMode = backdoorutils::getEnvBool("APOLLO_PROFILING", "");
	if (backdoorutils::getEnvBool("APOLLO_PROFILING", "--reindex_polyhedral_phases_detector", true)) {
		ProfilingPlugins.push_back(new ReIndexPhasesDetector(false));
	}
	if (backdoorutils::getEnvBool("APOLLO_PROFILING", "--reindex_polyhedral_phases_detector_full", true)) {
		ProfilingPlugins.push_back(new ReIndexPhasesDetector(true));
	}
	if (backdoorutils::getEnvBool("APOLLO_PROFILING", "--polyhedral_phases_detector", true)) {
		ProfilingPlugins.push_back(new LinearPhasesDetector(false));
	}
	if (backdoorutils::getEnvBool("APOLLO_PROFILING", "--polyhedral_phases_detector_full", true)) {
		ProfilingPlugins.push_back(new LinearPhasesDetector(true));
	}
	if (profilingMode && ProfilingPlugins.size() == 0) {
		std::cerr << "APOLLO_PROFILING defined to a wrong value" << std::endl;
		std::cerr << "  current value is: " << getenv("APOLLO_PROFILING")
				<< std::endl;
		std::cerr
				<< "  expected value is one of: --reindex_phases_detector, --reindex_phases_detector_full, --polyhedral_phases_detector, --polyhedral_phases_detector_full"
				<< std::endl;
		exit(-1);
	}
}

//Raquel new

void RuntimeConfig::setMode(int mode){
	switch (mode){
		case 0: { //Apollo normal behavior
			ExecutionMode.launchSeqThread = true;
			ExecutionMode.abortApollo = true;
			ExecutionMode.timeout = false;
			ExecutionMode.timeoutValue = -1;
			ExecutionMode.maxTime = -1;
			break;
		}
		case 1: { //Launch the sequential thread, but wait until the optimization has been computed
			ExecutionMode.launchSeqThread = true;
			ExecutionMode.abortApollo = false;
			ExecutionMode.timeout = false;
			ExecutionMode.timeoutValue = -1;
			ExecutionMode.maxTime = -1;
			break;
		}
		
		case 2: { //Don't even launch sequential thread and wait for the optimization to be computed
			ExecutionMode.launchSeqThread = false;
			ExecutionMode.abortApollo = false;
			ExecutionMode.timeout = false;
			ExecutionMode.timeoutValue = -1;
			ExecutionMode.maxTime = -1;
			break;
		}
		case 3: { //Set a timeout of 5x for the optimization computation to finish
			ExecutionMode.launchSeqThread = true;
			ExecutionMode.abortApollo = false;
			ExecutionMode.timeout = true;
			if(ExecutionMode.timeoutValue == -1){
				setDoubleFromEnv("APOLLO_TIMEOUT", ExecutionMode.timeoutValue);
				if(ExecutionMode.timeoutValue == -1){
					ExecutionMode.timeoutValue = 5;
				}
			}
			ExecutionMode.maxTime = -1;
			break;
		}
		case 4: { //Set a timeout value for the optimization computation to finish
			ExecutionMode.launchSeqThread = false;
			ExecutionMode.abortApollo = false;
			ExecutionMode.timeout = true;
			ExecutionMode.timeoutValue = -1;
			if(ExecutionMode.maxTime == -1){
				setDoubleFromEnv("APOLLO_TIMEOUT", ExecutionMode.maxTime);
				if(ExecutionMode.maxTime == -1){
					ExecutionMode.maxTime = 0.2;
				}
			}
			//printf("maxTimeout? %f\n", ExecutionMode.maxTime);
			break;
		}
		case 5: { //Run apollo as original, don't apply transformations
			ExecutionMode.launchSeqThread = false;
			ExecutionMode.abortApollo = false;
			ExecutionMode.timeout = false;
			ExecutionMode.timeoutValue = -1;
			ExecutionMode.maxTime = -1;
			launchOriginal = true;
			break;
		}
		default:
			assert(false && "seqThreadBehavior: invalid case.");
	}
	//printApolloMode();
}

void RuntimeConfig::createPlutoOptsSet(){	
	// Checker si csv existants sinon prendre celui par défaut
	// Lire csv et pour chaque ligne faire un push_back	
	// File pointer 
    fstream fin; 
  
    // Open an existing file 
    fin.open("./set.options", ios::in);
    if(!fin.is_open()){
        fin.open("./default.options", ios::in);
        cout << "set.options could not be found in the current directory, using default file: default.options\n";
    } 
  
    int count = 0;
  
    // read the data from the file 
    // as string vector 
    vector<int> row; 
    string line, word, temp; 
    while (getline(fin, line, ';')) { 
        row.clear();

		if(line.empty())
			break;

        // used for breaking words 
        stringstream s(line); 
  
        // read every column data of a row and 
        // store it in a string variable, 'word' 
        while (getline(s, word, ',')) { 
			if(!word.empty())
            	row.push_back(stoi(word)); 
        }

		if (row.size() < 13 || row[12]<1){
			cout << "Not enough tile sizes provided, option bypassed\n";
			break;
		}

		std::vector<int> tileSizes;
		for (auto it=row.begin()+13; it!=row.end(); ++it)
			tileSizes.push_back(*it);

		if (row[12] != tileSizes.size()){
			cout << "Size read is not the same as the effective size, option bypassed\n";
			break;
		}

        PlutoOptsSet.push_back(
            std::make_pair(generatePlutoOptions(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7], row[8], row[9], row[10], row[11]), 
            tileSizes));

        count++;
    } 
    if (count == 0) 
        cout << "Empty file loaded\n";
	else 
    	cout << count << " options loaded" << "\n";

    fin.close();
															  //parallel,  tile, intratileopt, l2tile, identity, rar, partlbtile, lbtile, fuse, unroll, unroll factor, useIsl,          tileSize, tileType//
	/*PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false, false,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //original code*/
	}
/*
// CC2020 transformation set
void RuntimeConfig::createPlutoOptsSet(){	
															   //parallel,  tile, intratileopt, l2tile, identity, rar, partlbtile, lbtile, fuse, unroll, unroll factor, useIsl,          tileSize, tileType//
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false, false,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //original code
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true, false,      false,    false,  true, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //fast pluto
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,       true,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //apollo original version
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true, false,       true,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //parallel, unroll by 2, intratile
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //parallel, unroll by 2, tile x 32

	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 64,     1))); //parallel, unroll by 2, tile x 64
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,       true,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 64,     1))); //parallel, unroll by 2, tile x 64

	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair(128,     1))); //parallel, unroll by 2, tile x 128
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,       true,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair(128,     1))); //parallel, unroll by 2, intratile, tile x 128

	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     0))); //parallel, tile x 32, 32, 64
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 64,     2))); //parallel, tile x 64, 64, 128
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     3))); //parallel, tile x 128, 128, 256

	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,      false,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair(256,     1))); //parallel, tile x 256
	// ========================================================================================================= diamond tile =============================================================================== //
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(      true,  true,       true,    false, false, false,      true,  true,       2, false,             2,  false), std::make_pair( 32,     1))); //parallel, tile x 32, partlbtile

	// ========================================================================================================== no parallel =============================================================================== //
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false,  true,       true,    false,  true, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //identity, tile x 32
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false,  true,       true,    false,  true, false,     false, false,       2, false,             2,  false), std::make_pair( 64,     1))); //identity, tile x 64
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false,  true,       true,    false,  true, false,     false, false,       2, false,             2,  false), std::make_pair(128,     1))); //identity, tile x 128
	PlutoOptsSet.push_back(std::make_pair(generatePlutoOptions(     false,  true,       true,    false, false, false,     false, false,       2, false,             2,  false), std::make_pair( 32,     1))); //tile x 32
}*/          

void RuntimeConfig::printApolloMode(){
	printf("SET MODE: launchSeqThread? %d, abortApollo? %d, timeout? %d, timeoutValue = %f maxTime = %f \n", ExecutionMode.launchSeqThread, ExecutionMode.abortApollo, ExecutionMode.timeout, ExecutionMode.timeoutValue, ExecutionMode.maxTime);

}

void RuntimeConfig::setApolloAsSlave(){
	apolloAsSlave = true;
}
/*
int RuntimeConfig::registerDataflowThread(){
	NumberOfDataflowThreads++;
	if(NumberOfDataflowThreads != 0){
		NumberOfThreads = omp_get_max_threads()/NumberOfDataflowThreads;
	}else{
		NumberOfThreads = omp_get_max_threads();
	}
	//printf("Threads = %d, dataflow threads = %d\n", NumberOfThreads, NumberOfDataflowThreads);
	return NumberOfDataflowThreads;
}

int RuntimeConfig::removeDataflowThread(){
	NumberOfDataflowThreads--;
	if(NumberOfDataflowThreads != 0){
		NumberOfThreads = omp_get_max_threads()/NumberOfDataflowThreads;
	}else{
		NumberOfThreads = omp_get_max_threads();
	}
	return NumberOfDataflowThreads;
}
*/
RuntimeConfig::Ehrhart RuntimeConfig::computeNumberOfUnknowns(int64_t currentLoopId, int d, bool restart){
	int unknowns;
	auto it = loopDepths.find(currentLoopId);
	Ehrhart polynomial;
	if(it == loopDepths.end()){
		unknowns = 0.5*d*d + 1.5*d + 1;
		polynomial.d = d;
		polynomial.unknowns = unknowns;
		polynomial.compStart = true;
		polynomial.compEnd = false;
		polynomial.dynamic = false;
		polynomial.notLinear = false;
		polynomial.solved = false;
		polynomial.C_ij = new double[unknowns];
		loopDepths.insert(std::make_pair(currentLoopId, polynomial));
		if(d > InstrumentationChunkSize){
			InstrumentationChunkSize = d;
			InstrumentationSample = InstrumentationChunkSize;
		}
		//ehrhartCompStart = true;
		//printf("Number of unknowns: %d\n", unknowns);
	}else{
		if(restart){
			it->second.dynamic = false;
		}
		polynomial = it->second;
		//printf("Already a table --> Number of unknowns: %d\n", it->second.unknowns);
	}
	return polynomial;
}

RuntimeConfig::~RuntimeConfig() {
	
	printf("RuntimeConfig destructor... \n");
	printf("TxTableLoop size = %lu, Number of threads= %d\n", TxTableLoop.size(), NumberOfThreads);
	for ( auto iter = TxTableLoop.begin(); iter != TxTableLoop.end(); ++iter ){
		printf("TableLoop id %s\n", iter->first.c_str());
  		for (std::vector<apollo::TxInfo>::iterator iterate = iter->second.begin() ; iterate != iter->second.end(); ++iterate){
  			std::string actor_id = iterate->getActorId();
		    int loop_id = iterate->getLoopId();
		    double plutoTime = iterate->getPlutoTime();
		    double optTime = iterate->getOptTimePerIter();
		    std::string opts = iterate->getPlutoOptions();
		    printf("Thread id: %lu, idChunk = %d\n", pthread_self(), iterate->getIdChunk());
		    printf("TxTableLoop: loop_id: %d, actor_id: %s, Pluto_opts: %s, pluto_time: %f,  optIter_time: %f, Lower: %ld, Upper: %ld, is Original?: %d\n", 
		    	loop_id, actor_id.c_str(), opts.c_str(), plutoTime, optTime, iterate->getLower(), iterate->getUpper(), iterate->isOriginal());
		    printf("PlutoOptsAnalyzed set size: %d\n", iterate->getSizePlutoOptsAnalyzed());
		    std::unordered_set<std::string> options = iterate->getPlutoOptsAnalyzed();
		    for(auto it = options.begin(); it!= options.end(); ++it){
				printf("Pluto options analyzed: %s\n", it->c_str());
			}
  		}
	    
	}

	for ( auto it = loopDepths.begin(); it != loopDepths.end(); ++it ){
		printf("loopDepths id %lu\n", it->first);
		printf("Number of unknowns: %d\n", it->second.unknowns);
	}

	printf("Freeing memory...");

	for (std::pair<PlutoOptions*, std::vector<int>> it : PlutoOptsSet){
		pluto_options_free(it.first);
	}

	printf("memory fried...");
	
	TxTableLoop.clear();
	loopDepths.clear();
	//printf("RuntimeConfig destructor finished\n");
}
//end Raquel new