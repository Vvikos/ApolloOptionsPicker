#ifdef __cplusplus
#include <cstdint>
#include <string>
extern "C" {
#endif
void test();
void initApolloForDataflow();
void disableApollo();
void apolloSetMode(int mode, double timeMax, int timeout);
void apolloSetTxsToTry(int numTxs);
void apolloEnableMultiversioning();
void apolloDisableMultiversioning();
void apolloEnableParallel();
void apolloDisableParallel();
void apolloEnableMultithreading();
void apolloDisableMultithreading();
void apolloAddActorConfig(int isDynamic, unsigned long threadId, const char* actorName);
void apolloAddJobIdx(int jobId, unsigned long threadId);
void apolloStopTxSelection(unsigned long threadId, const char* actorName);
double apolloGetEstimatedTime(unsigned long threadId, const char* actorName);
#ifdef __cplusplus
void apolloAddDynamicParams(std::int64_t* params, int numberOfParams);
}
#endif
