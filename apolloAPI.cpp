#include "apolloAPI.h"
#include "RuntimeConfig.h"

extern RuntimeConfig apolloRuntimeConfig;
extern std::mutex apolloRuntimeConfigMutex;

void test(){
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~TESTING THIS~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	apolloRuntimeConfig.printApolloMode();
}

void initApolloForDataflow(){
	apolloRuntimeConfigMutex.lock();
	apolloSetMode(4, 50, -1);
	apolloSetTxsToTry(14);
	apolloRuntimeConfig.createPlutoOptsSet();
	apolloDisableParallel();
	apolloEnableMultiversioning();
	apolloEnableMultithreading();
	apolloRuntimeConfigMutex.unlock();
	//test();
}

void disableApollo(){
	apolloDisableMultiversioning();
	apolloSetMode(5, -1, -1);
}

void apolloSetMode(int mode, double timeMax, int timeout){
	apolloRuntimeConfig.setApolloAsSlave();
	if(mode>=0 && mode<=5){
		apolloRuntimeConfig.setMode(mode);
	}else{
		apolloRuntimeConfig.setMode(0);
	}
	if(mode == 3){
		apolloRuntimeConfig.ExecutionMode.timeout = timeout;
	}else if(mode == 4){
		apolloRuntimeConfig.ExecutionMode.maxTime = timeMax;
	}
}

void apolloSetTxsToTry(int numTxs){
	apolloRuntimeConfig.TxsToTry = numTxs;
}

void apolloEnableMultiversioning(){
	apolloRuntimeConfig.multiVersioning = true;
}

void apolloDisableMultiversioning(){
	apolloRuntimeConfig.multiVersioning = false;
}

void apolloEnableParallel(){
	for (std::pair<PlutoOptions*, std::vector<int>> it : apolloRuntimeConfig.PlutoOptsSet){
		it.first->parallel = true;
	}
}

void apolloDisableParallel(){
	for (std::pair<PlutoOptions*, std::vector<int>> it : apolloRuntimeConfig.PlutoOptsSet){
		it.first->parallel = false;
	}
}

void apolloEnableMultithreading(){
	apolloRuntimeConfig.multiThreading = true;
}
void apolloDisableMultithreading(){
	apolloRuntimeConfig.multiThreading = false;
}

void apolloAddActorConfig(int isDynamic, unsigned long threadId, const char* actorName){
	RuntimeConfig::Actor actorInfo;
	std::string aux(actorName);
	actorInfo.actorName = aux;
	actorInfo.active =  true;
	//apolloRuntimeConfig.setApolloAsSlave();
	if(isDynamic == 0){ // static actors
		actorInfo.isDynamic = false;
	}else if(isDynamic == 1){ //dynamic actors
		actorInfo.isDynamic = true;
	}else{
		printf("Warning: actor info not properly configured: is it static or dynamic?\n");
	}
	/*if(numberOfParams != 0){
		std::set<std::int64_t> dynParams(params, params+numberOfParams);
		actorInfo.dynamicParams = dynParams;
		for(auto i : actorInfo.dynamicParams) {
		  actorInfo.paramCombination += std::to_string(i);
		}
	}else{
		actorInfo.isDynamic = false;
		actorInfo.paramCombination = std::to_string(-1);
	}*/
	bool done = false;
	apolloRuntimeConfigMutex.lock();
	auto it = apolloRuntimeConfig.threadActor.find(threadId);
	if(it != apolloRuntimeConfig.threadActor.end()){
		for (std::vector<RuntimeConfig::Actor>::iterator n = it->second.begin() ; n != it->second.end(); ++n){
	        if(aux == n->actorName){
	        	n->active = true;
	        	//n->paramCombination = actorInfo.paramCombination;
	        	//n->dynamicParams = actorInfo.dynamicParams;
				n->numberOfLoops = actorInfo.numberOfLoops;
				n->isDynamic = actorInfo.isDynamic;
				n->useApollo = false;
				done = true;
			}else{
				n->active = false;
				n->useApollo = false;
			}
	    }
	    if(!done){
			it->second.push_back(actorInfo);
		}
	}else{
		std::vector<RuntimeConfig::Actor> actorSet;
		actorSet.push_back(actorInfo);
		apolloRuntimeConfig.threadActor.insert(std::make_pair(threadId, actorSet));
	}

	apolloRuntimeConfigMutex.unlock();

}

void apolloAddDynamicParams(std::int64_t* params, int numberOfParams){
	std::string paramCombination;
	for(int i = 0; i < numberOfParams; i++){
		paramCombination += std::to_string(params[i]);
	}
	apolloRuntimeConfigMutex.lock();
	apolloRuntimeConfig.dynamicParams = paramCombination;
	apolloRuntimeConfigMutex.unlock();
}

void apolloAddJobIdx(int jobId, unsigned long threadId){
	std::string jobIdx = std::to_string(jobId);
	apolloAddActorConfig(1, threadId, jobIdx.c_str());
}

void apolloStopTxSelection(unsigned long threadId, const char* actorName){
	std::string aux(actorName);
	apolloRuntimeConfigMutex.lock();
	auto it = apolloRuntimeConfig.threadActor.find(threadId);
	if(it != apolloRuntimeConfig.threadActor.end()){
		 for (std::vector<RuntimeConfig::Actor>::iterator n = it->second.begin() ; n != it->second.end(); ++n){
			if(aux == n->actorName){
				n->stopTxSelection = true;
			}
		}
	}else{
		printf("Warning: this thread is not linked to an actor\n");
	}
	apolloRuntimeConfigMutex.unlock();
}

double apolloGetEstimatedTime(unsigned long threadId, const char* actorName){ //FIX THIS!!!!
	apolloRuntimeConfigMutex.lock();
	double estimatedTime = 0;
	std::string aux(actorName);
	std::string partialUniqueId;
	auto it = apolloRuntimeConfig.threadActor.find(threadId);
	if(it != apolloRuntimeConfig.threadActor.end()){
		for (std::vector<RuntimeConfig::Actor>::iterator n = it->second.begin() ; n != it->second.end(); ++n){
			if(aux == n->actorName){
				//partialUniqueId = n->paramCombination + "_" + n->actorName;
				if(n->isDynamic){
	        		partialUniqueId = apolloRuntimeConfig.dynamicParams + "_" + n->actorName;
	        	}else{
	        		partialUniqueId = "static_" + n->actorName;
	        	}
				for ( auto iter = apolloRuntimeConfig.TxTableLoop.begin(); iter != apolloRuntimeConfig.TxTableLoop.end(); ++iter ){
					std::size_t found = iter->first.find(partialUniqueId);
	  				if (found!=std::string::npos){
	  					for ( auto i = iter->second.begin(); i != iter->second.end(); ++i ){
	  						if(i->getOptTimePerIter() != -1 && i->getMaxIters() != 0){
	  							estimatedTime = estimatedTime + (i->getOptTimePerIter())*(i->getMaxIters());
	  							//printf("Estimated time: %f\n", estimatedTime);
	  						}
	  					}
	  				}
				}
			}
		}
	}else{
		printf("Warning: this thread is not linked to an actor\n");
	}
	apolloRuntimeConfigMutex.unlock();

	if(estimatedTime != -1){
		estimatedTime = estimatedTime/1000000;
	}

	return estimatedTime;
}



