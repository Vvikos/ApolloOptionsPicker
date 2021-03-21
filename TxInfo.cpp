

#include "TxSelection/TxInfo.h"
#include "CodeBones/CodeBones.h"

using namespace apollo;

apollo::TxInfo::TxInfo(std::string actorId, int64_t loopId){
	//this->PlutoOpt = pluto_options_alloc();
	this->actorId = actorId;
	this->loopId = loopId;
	this->plutoTime = -1;
	this->loopTime = -1;
	this->optChunkTime = -1;
	this->optTimePerIter = -1;
	this->ChunkNumberOfIterations = -1;
	this->idChunk = -1;
	this->copyableCodeGen = false;
	this->storedPM = false;
	this->isOriginalChunk = false;
	this->isOrigTx = false;

	this->Lower = 0;
	this->Upper = 0;

	this->maxIters = 0;
	
	this->maxBound = std::numeric_limits<long>::max() - 1;

	this->Verification = nullptr;
	this->Optimized = nullptr;
	this->CodeGen = nullptr;
	this->PM = nullptr;
	this->NewPhiState = nullptr;

	this->RawSkeletons = nullptr;
	this->NumberOfSkeletons = -1;
}

apollo::TxInfo::TxInfo(struct CodeGenerator::StaticSkeletonInfo *Skeletons, unsigned NumSkeletons){
	this->actorId = -1;
	this->loopId = -1;
	this->plutoTime = -1;
	this->loopTime = -1;
	this->ChunkNumberOfIterations = -1;
	this->idChunk = -1;
	this->copyableCodeGen = false;
	this->storedPM = false;
	this->isOriginalChunk = false;
	this->isOrigTx = false;

	this->Lower = 0;
	this->Upper = 0;

	this->maxIters = 0;

	this->maxBound = std::numeric_limits<long>::max() - 1;

	this->Verification = nullptr;
	this->Optimized = nullptr;
	this->PM = nullptr;
	this->NewPhiState = nullptr;
	this->InstrumentationResults = nullptr;


	this->RawSkeletons = Skeletons;
	this->NumberOfSkeletons = NumSkeletons;
	this->CodeGen = new CodeGenerator(RawSkeletons, NumberOfSkeletons);
}

apollo::TxInfo::TxInfo(){     // default construct
	this->actorId = -1;
	this->loopId = -1;
	this->plutoTime = -1;
	this->loopTime = -1;
	this->optChunkTime = -1;
	this->optTimePerIter = -1;
	this->ChunkNumberOfIterations = -1;
	this->idChunk = -1;
	this->copyableCodeGen = false;
	this->storedPM = false;
	this->isOriginalChunk = false;
	this->isOrigTx = false;

	this->Lower = 0;
	this->Upper = 0;

	this->maxIters = 0;

	this->maxBound = std::numeric_limits<long>::max() - 1;

	this->Verification = nullptr;
	this->Optimized = nullptr;
	this->CodeGen = nullptr;
	this->PM = nullptr;
	this->NewPhiState = nullptr;
	this->InstrumentationResults = nullptr;

	this->RawSkeletons = nullptr;
	this->NumberOfSkeletons = -1;
}
//Copy constructor
apollo::TxInfo::TxInfo(const TxInfo &Tx){
	//printf("Copy constructor\n");
	this->actorId = Tx.actorId;
	this->loopId = Tx.loopId;
	this->uniqueId = Tx.uniqueId;
	this->plutoTime = Tx.plutoTime;
	this->loopTime = Tx.loopTime;
	this->optPluto = Tx.optPluto;
	this->optChunkTime = Tx.optChunkTime;
	this->optTimePerIter = Tx.optTimePerIter;
	this->maxBound = Tx.maxBound;
	this->maxIters = Tx.maxIters;
	this->ChunkNumberOfIterations = Tx.ChunkNumberOfIterations;
	this->idChunk = Tx.idChunk;
	this->copyableCodeGen = false;
	this->storedPM = Tx.storedPM;
	this->isOriginalChunk = Tx.isOriginalChunk;
	this->isOrigTx = Tx.isOrigTx;
	this->PlutoOptsAnalyzed = Tx.PlutoOptsAnalyzed;
	this->Lower = Tx.Lower;
	this->Upper = Tx.Upper;
	this->ExtremeBounds = Tx.ExtremeBounds;
	this->ExtremeMemRanges = Tx.ExtremeMemRanges;
	//this->InstrumentationResults = new InstrumentationResult(*Tx.InstrumentationResults);

	this->NumberOfSkeletons = Tx.NumberOfSkeletons;

	//this->InstrumentationResults = (Tx.InstrumentationResults == nullptr) ? nullptr : new InstrumentationResult(*Tx.InstrumentationResults);

	this->PM = (Tx.PM == nullptr) ? nullptr : new PredictionModel(*Tx.PM);

	this->InstrumentationResults = (Tx.InstrumentationResults == nullptr) ? nullptr : new InstrumentationResult(*Tx.InstrumentationResults);

	this->NewPhiState = (Tx.NewPhiState == nullptr) ? nullptr : new apollo::PhiState(*Tx.NewPhiState);

	this->RawSkeletons = (Tx.RawSkeletons == nullptr) ? nullptr : Tx.RawSkeletons;

	if(Tx.CodeGen == nullptr){
	this->CodeGen = nullptr;
	}else{
		this->CodeGen = new CodeGenerator(RawSkeletons, NumberOfSkeletons);
		if(Tx.copyableCodeGen){
			Tx.CodeGen->moveJitState(this->CodeGen,this->Verification,this->Optimized);
			this->copyableCodeGen = true;
		}
	}
}

apollo::TxInfo::~TxInfo(){

}

bool apollo::TxInfo::operator==(const TxInfo& Tx){
	bool ret;
	if(this->actorId == Tx.actorId && this->loopId == Tx.loopId && this->optPluto == Tx.optPluto){
		ret = true;
	}else{
		ret = false;
	}
	return ret; 
}

void apollo::TxInfo::restartTxInfo(){
	this->plutoTime = -1;
	this->loopTime = -1;
	this->optChunkTime = -1;
	this->optTimePerIter = -1;
	this->ChunkNumberOfIterations = -1;
	this->idChunk = -1;
	this->copyableCodeGen = false;
	this->storedPM = false;
	this->isOriginalChunk = false;
	this->isOrigTx = false;
	this->PlutoOptsAnalyzed.clear();

	this->Lower = 0;
	this->Upper = 0;

	this->maxIters = 0;

	this->maxBound = std::numeric_limits<long>::max() - 1;

	this->Verification = nullptr;
	this->Optimized = nullptr;
	this->CodeGen = nullptr;
	this->PM = nullptr;
	this->NewPhiState = nullptr;
	this->InstrumentationResults = nullptr;

	this->CodeGen = new CodeGenerator(RawSkeletons, NumberOfSkeletons);
}

void apollo::TxInfo::dumpTx(){
	printf("actorId: %s uniqueId: %s optPluto: %s, idChunk: %d\n", actorId.c_str(), uniqueId.c_str(), optPluto.c_str(), idChunk);
	printf("loopId: %lu plutoTime: %f optChunkTime: %f optTimePerIter: %f loopTime: %f maxIters: %f\n", loopId, plutoTime, optChunkTime, optTimePerIter, loopTime, maxIters);
	printf("ChunkNumberOfIterations: %f Lower: %lu Upper: %lu maxBound: %lu, NumberOfSkeletons: %lu\n", ChunkNumberOfIterations, Lower, Upper, maxBound, NumberOfSkeletons);
	printf("copyableCodeGen: %d storedPM: %d isOriginalChunk: %d isOrigTx: %d\n", copyableCodeGen, storedPM, isOriginalChunk, isOrigTx);

	(this->PM == nullptr) ? printf("No PM\n") : printf("PM stored \n");

	(this->InstrumentationResults == nullptr) ? printf("No InstrRes\n") : printf("InstrRes stored \n");

	(this->NewPhiState == nullptr) ? printf("No PhiSt\n") : printf("PhiSt stored \n");

	(this->RawSkeletons == nullptr) ? printf("No RawSk\n") : printf("RawSk stored\n");

	(this->CodeGen == nullptr) ? printf("No CodeGen\n") : printf("CodeGen stored\n");

	(this->Verification == nullptr) ? printf("No Verification\n") : printf("Verification stored\n");

	(this->Optimized == nullptr) ? printf("No Optimized\n") : printf("Optimized stored\n");
}

std::string apollo::TxInfo::getActorId(){
	return this->actorId;
}

void apollo::TxInfo::setActorId(std::string actorId){
	this->actorId = actorId;
}

int64_t apollo::TxInfo::getLoopId(){
	return this->loopId;
}

void apollo::TxInfo::setLoopId(int64_t loopId){
	this->loopId = loopId;
}

std::string apollo::TxInfo::getUniqueId(){
	return this->uniqueId;
}

void apollo::TxInfo::setUniqueId(std::string id){
	this->uniqueId = id;
}

bool apollo::TxInfo::isCopyable(){
	return this->copyableCodeGen;
}

void apollo::TxInfo::makeCopyable(){
	this->copyableCodeGen = true;
}

void apollo::TxInfo::setLower(long Lower){
	this->Lower = Lower;
}

long apollo::TxInfo::getLower(){
	return this->Lower;
}

void apollo::TxInfo::setUpper(long Upper){
	this->Upper = Upper;
}

long apollo::TxInfo::getUpper(){
	return this->Upper;
}

void apollo::TxInfo::setIdChunk(int id){
	this->idChunk = id;
}
int apollo::TxInfo::getIdChunk(){
	return this->idChunk;
}

bool apollo::TxInfo::isPMstored(){
	return this->storedPM;
}

void apollo::TxInfo::setOriginalFlag(){
	this->isOriginalChunk = true;
}

bool apollo::TxInfo::isOriginal(){
	return this->isOriginalChunk;
}

void apollo::TxInfo::setTryingOrig(){
	this->isOrigTx = true;
}

bool apollo::TxInfo::isTryingOrig(){
	return this->isOrigTx;
}

double apollo::TxInfo::getMaxIters(){
	return this->maxIters;
}

void apollo::TxInfo::setMaxIters(double iters){
	this->maxIters = iters;
}

void apollo::TxInfo::addPlutoOptsAnalyzed(std::string opts){
	auto iter = PlutoOptsAnalyzed.find(opts);
    if(iter != PlutoOptsAnalyzed.end()){
      printf("Transformation already tried: %s\n", opts.c_str());
    }else{
      PlutoOptsAnalyzed.insert(opts);
      //printf("another combination inserted... plutoOptsAnalyzed size: %lu\n", PlutoOptsAnalyzed.size());
    }
}

bool apollo::TxInfo::txAlreadyAnalyzed(std::string opts){
	auto iter = PlutoOptsAnalyzed.find(opts);
    if(iter != PlutoOptsAnalyzed.end()){
      return true;
    }else{
      return false;
    }
}

std::unordered_set<std::string> apollo::TxInfo::getPlutoOptsAnalyzed(){
	return this->PlutoOptsAnalyzed;
}

void apollo::TxInfo::setPlutoOptsAnalyzed(std::unordered_set<std::string> &options){
	this->PlutoOptsAnalyzed = options;
	//printf("Copy pluto options, size = %lu\n", PlutoOptsAnalyzed.size());
}

int apollo::TxInfo::getSizePlutoOptsAnalyzed(){
	return this->PlutoOptsAnalyzed.size();
}

void apollo::TxInfo::storePlutoOptions(PlutoOptions *PlutoOpts, std::vector<int> tileSizes){
	//this->PlutoOpt = PlutoOpts;
	/*parallel, tile, intratileopt, l2tile, identity, rar, partlbtile, fuse, unroll, unroll factor, useIsl*/
	std::string options;
	options  = std::to_string(PlutoOpts->parallel);
	options += std::to_string(PlutoOpts->tile);
	options += std::to_string(PlutoOpts->intratileopt);
	options += std::to_string(PlutoOpts->l2tile);
	options += std::to_string(PlutoOpts->identity);
	options += std::to_string(PlutoOpts->rar);
	options += std::to_string(PlutoOpts->partlbtile);
	options += std::to_string(PlutoOpts->lbtile);
	options += std::to_string(PlutoOpts->fuse);
	options += std::to_string(PlutoOpts->unroll);
	options += std::to_string(PlutoOpts->ufactor);
	options += std::to_string(PlutoOpts->isldep);
	options += std::to_string(tileSizes.size());
	for (auto it=tileSizes.begin(); it!=tileSizes.end(); ++it)
		options += std::to_string(*it);

	this->optPluto = options;
	//this->optPluto = options + std::to_string(rand() % 1000);
}

std::string apollo::TxInfo::plutoOptionsToString(PlutoOptions *PlutoOpts, std::vector<int> tileSizes){
	std::string options;
	options  = std::to_string(PlutoOpts->parallel);
	options += std::to_string(PlutoOpts->tile);
	options += std::to_string(PlutoOpts->intratileopt);
	options += std::to_string(PlutoOpts->l2tile);
	options += std::to_string(PlutoOpts->identity);
	options += std::to_string(PlutoOpts->rar);
	options += std::to_string(PlutoOpts->partlbtile);
	options += std::to_string(PlutoOpts->lbtile);
	options += std::to_string(PlutoOpts->fuse);
	options += std::to_string(PlutoOpts->unroll);
	options += std::to_string(PlutoOpts->ufactor);
	options += std::to_string(PlutoOpts->isldep);
	options += std::to_string(tileSizes.size());
	for (auto it=tileSizes.begin(); it!=tileSizes.end(); ++it)
		options += std::to_string(*it);
	return options;
}

void apollo::TxInfo::setPlutoOptions(std::string opts){
	this->optPluto = opts;
}

void apollo::TxInfo::setPlutoTime(double time){
	this->plutoTime = time;
}

void apollo::TxInfo::setOptChunkTime(double time){
	this->optChunkTime = time;
}

double apollo::TxInfo::getPlutoTime(){
	return this->plutoTime;
}

double apollo::TxInfo::getOptChunkTime(){
	return this->optChunkTime;
}

void apollo::TxInfo::setOptTimePerIter(double time, double iters){
	double timePerIter = (time*1000000000)/iters; //in ns
	this->optTimePerIter = timePerIter;
	//printf("Time added ----> chunkTime = %f, iters = %f, timePerIter = %f us, ...... thread_id =%lu\n", time, iters, timePerIter, pthread_self());
}

void apollo::TxInfo::setOptTimePerIter(double timePerIter){
	this->optTimePerIter = timePerIter;
}

double apollo::TxInfo::getOptTimePerIter(){
	return this->optTimePerIter;
}

std::string apollo::TxInfo::getPlutoOptions(){
	return this->optPluto;
}

std::string apollo::TxInfo::plutoOptionsToString(){
	return this->optPluto;
}

void apollo::TxInfo::setLoopTime(double time){
	this->loopTime = time;
}

double apollo::TxInfo::getLoopTime(){
	return this->loopTime;
}

void apollo::TxInfo::storePM(APOLLO_PROG *apolloProg){
	this->PM = new PredictionModel(*apolloProg->PredictionModel);
	this->storedPM = true;
	//printf("From stored PM: %d\n", PM->randomValue);
}

void apollo::TxInfo::retrievePM(APOLLO_PROG *apolloProg){
	apolloProg->PredictionModel = std::make_shared<PredictionModel>(*PM);
	//printf("From stored PM: %d\n", PM->randomValue);
}

void apollo::TxInfo::storeInstrumentationResults(APOLLO_PROG *apolloProg){
	this->InstrumentationResults = nullptr;
	this->InstrumentationResults = new InstrumentationResult(*apolloProg->InstrumentationResults);
	//InstrumentationResults->dump(std::cout);
}

void apollo::TxInfo::retrieveInstrumentationResults(APOLLO_PROG *apolloProg){
	apolloProg->InstrumentationResults = std::make_shared<InstrumentationResult>(*InstrumentationResults);
}

void apollo::TxInfo::storeExtremeBoundsAndMemoryRanges(APOLLO_PROG *apolloProg){
	this->ExtremeBounds = apolloProg->ExtremeBounds;
	this->ExtremeMemRanges = apolloProg->ExtremeMemRanges;
}

void apollo::TxInfo::retrieveExtremeBoundsAndMemoryRanges(APOLLO_PROG *apolloProg){
	apolloProg->ExtremeBounds = this->ExtremeBounds;
	apolloProg->ExtremeMemRanges = this->ExtremeMemRanges;
}

void apollo::TxInfo::storePhiState(apollo::PhiState &PhiStateObj){
	this->NewPhiState = new apollo::PhiState(PhiStateObj);
}

void apollo::TxInfo::storeChunkNumberOfIterations(double ChunkNumberOfIters){
	this->ChunkNumberOfIterations = ChunkNumberOfIters;
}

double apollo::TxInfo::getChunkNumberOfIterations(){
	return this->ChunkNumberOfIterations;
}

void apollo::TxInfo::storeMaxBound(long previousMaxBound){
	this->maxBound = previousMaxBound;
}

long apollo::TxInfo::getMaxBound(){
	return this->maxBound;
}

void apollo::TxInfo::updateJitState(TxInfo &Tx){
	Tx.CodeGen->moveJitState(this->CodeGen, this->Verification, this->Optimized);
	this->copyableCodeGen = true;
}