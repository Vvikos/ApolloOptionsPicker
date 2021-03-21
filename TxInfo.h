

#ifndef TXINFO_H
#define TXINFO_H

#include <string>
#include <set>
#include <memory>

// pluto
#include "osl/scop.h"
#include "pluto/libpluto.h"

#include <JitCompiler/CodeGenerator.h>


struct ScopTypeStruct;

namespace apollo{

class TxInfo{

public:
	using CodeBonesFunTy = long (*)(ParamTy, PhiStateTy, void *, long, long);

 	struct JitState {
    	std::unique_ptr<llvm::Module> PatchedModule;
    	std::unique_ptr<llvm::ExecutionEngine> ExecutionEngine;
    	CodeBonesFunTy VerificationFunction;
    	CodeBonesFunTy OptimizedFunction;
  	};

  	CodeBonesFunTy Verification;
  	CodeBonesFunTy Optimized;

	apollo::CodeGenerator *CodeGen;
	PredictionModel *PM;
	apollo::PhiState *NewPhiState;

	InstrumentationResult *InstrumentationResults;

	std::vector<BoundRangeTy> ExtremeBounds;
  	std::vector<MemRangeTy> ExtremeMemRanges;

	struct CodeGenerator::StaticSkeletonInfo *RawSkeletons;
	long NumberOfSkeletons;

	double ChunkNumberOfIterations;
	long Lower;
	long Upper;

private:
	std::string optPluto;
	std::string actorId;
	std::string uniqueId;
	int64_t loopId;
	double plutoTime;
	double optChunkTime;
	double optTimePerIter;
	double loopTime;
	double maxIters;
	long maxBound;
	bool copyableCodeGen;
	bool storedPM;
	int idChunk;
	bool isOriginalChunk;
	bool isOrigTx;
	std::unordered_set<std::string> PlutoOptsAnalyzed;

public:
	TxInfo(std::string actorId, int64_t loopId);
	TxInfo(struct CodeGenerator::StaticSkeletonInfo *Skeletons, unsigned NumSkeletons);
	TxInfo();  //default constructor
	TxInfo(const TxInfo &Tx);  //copy constructor
	~TxInfo(); //destructor
	std::string getActorId();
	void setActorId(std::string actorId);
	int64_t getLoopId();
	void setLoopId(int64_t loopId);
	void setUniqueId(std::string id);
	std::string getUniqueId();
	void storePlutoOptions(PlutoOptions *PlutoOpts, std::vector<int> tileSizes);
	std::string plutoOptionsToString(PlutoOptions *PlutoOpts, std::vector<int> tileSizes);
	void setPlutoOptions(std::string opts);
	void setPlutoTime(double time);
	void setOptChunkTime(double time);
	double getPlutoTime();
	double getOptChunkTime();
	bool isCopyable();
	void makeCopyable();
	void setOriginalFlag();
	bool isOriginal();
	void setTryingOrig();
	bool isTryingOrig();

	void restartTxInfo();
	void dumpTx();

	void setLower(long Lower);
	long getLower();
	void setUpper(long Upper);
	long getUpper();
	void setIdChunk(int id);
	int getIdChunk();

	bool isPMstored();

	void addPlutoOptsAnalyzed(std::string opts);
	bool txAlreadyAnalyzed(std::string opts);
	std::unordered_set<std::string> getPlutoOptsAnalyzed();
	void setPlutoOptsAnalyzed(std::unordered_set<std::string> &options);
	int getSizePlutoOptsAnalyzed();

	void setOptTimePerIter(double time, double iters);
	void setOptTimePerIter(double timePerIter);
	double getOptTimePerIter();

	double getMaxIters();
	void setMaxIters(double iters);

	std::string getPlutoOptions();
	std::string plutoOptionsToString();
	void setLoopTime(double time);
	double getLoopTime();

	bool operator==(const TxInfo& Tx);

	void storePM(APOLLO_PROG *apolloProg);
	void retrievePM(APOLLO_PROG *apolloProg);

	void storeInstrumentationResults(APOLLO_PROG *apolloProg);
	void retrieveInstrumentationResults(APOLLO_PROG *apolloProg);

	void storeExtremeBoundsAndMemoryRanges(APOLLO_PROG *apolloProg);
	void retrieveExtremeBoundsAndMemoryRanges(APOLLO_PROG *apolloProg);

	void storePhiState(apollo::PhiState &PhiStateObj);

	void storeChunkNumberOfIterations(double ChunkNumberOfIters);
	double getChunkNumberOfIterations();

	void storeMaxBound(long previousMaxBound);
	long getMaxBound();
	void updateJitState(TxInfo &Tx);
};

}

#endif
