//===--- CodeBones.cpp ----------------------------------------------------===//
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

#include <JitCompiler/CodeGenerator.h>
#include "CodeBones/CodeBones.h"
#include "Backdoor/Backdoor.h"
#include "CodeBones/BonesInfo/BoneComposedStmt.h"
#include "CodeBones/BonesToScop/BoneToAccess.h"
#include "CodeBones/BonesToScop/BoneToBody.h"
#include "CodeBones/BonesToScop/BoneToDomain.h"
#include "CodeBones/BonesToScop/BoneToScattering.h"
#include "CodeBones/AutoTile.h"
#include "APOLLO_PROG.h"
#include "RuntimeConfig.h"

#include <cstdio>
#include <cstring>
#include <string>

// openscop
#include "osl/scop.h"
#include "osl/statement.h"
#include "osl/body.h"
#include "osl/relation.h"
#include "osl/macros.h"
#include "osl/int.h"

#include "osl/extensions/arrays.h"
#include "osl/extensions/scatnames.h"
#include "osl/extensions/loop.h"
#include "osl/extensions/pluto_unroll.h"

// candl
extern "C" {
#include "candl/candl.h"
#include "candl/scop.h"
#include "candl/dependence.h"
#include "candl/options.h"
}

// pluto
#include "pluto/libpluto.h"

// cloog
#define CLOOG_INT_LONG 1
#include "cloog/cloog.h"

using namespace apollo;
using namespace std::placeholders;

using apollo::codebones::BoneComposedStmt;
using apollo::codebones::BoneStmt;
using apollo::codebones::BoneLoop;
using apollo::codebones::BoneAst;
using apollo::codebones::BoneAstDump;

//std::mutex codeGenMutex;
//pthread_mutex_t codeGenMutex = PTHREAD_MUTEX_INITIALIZER;
extern RuntimeConfig apolloRuntimeConfig;
extern std::mutex apolloRuntimeConfigMutex;

// this is an example of apossible AST optimization done with code-bones.
static void simplifyBoneNest(BoneLoop *Loop) {
  size_t BoneIdx = 0;
  size_t NumChildren = Loop->children().size();
  while (BoneIdx + 1 < NumChildren) {
    // we're going to be modifying this list! keep it inside the loop
    auto Children = Loop->children();
    BoneAst *ABone = Children[BoneIdx];
    BoneAst *NextBone = Children[BoneIdx + 1];
    if (!ABone->isStmtBone() || !NextBone->isStmtBone()) {
      BoneIdx++;
      continue;
    }
    BoneStmt *AStmtBone = (BoneStmt *)ABone;
    BoneStmt *NextStmtBone = (BoneStmt *)NextBone;
    // fuse bones of the same type
    if (false &&
        AStmtBone->isVerificationOnly() != NextStmtBone->isVerificationOnly()) {
      BoneIdx++;
      continue;
    }
    BoneComposedStmt::compose(AStmtBone, NextStmtBone,
                              BoneComposedStmt::CompositionType::Sequential);
    NumChildren--;
  }
  auto Children = Loop->children();
  for (size_t i = 0; i < Children.size(); ++i) {
    if (Children[i]->isLoop())
      simplifyBoneNest((BoneLoop *)Children[i]);
  }
}

apollo::CodeBoneManager::CodeBoneManager(APOLLO_PROG *apolloProg,
                                         apollo::CodeGenerator &CG)
  : CodeGen(CG) {
  traceEnter("apollo::code_bone_manager::code_bone_manager()");
  this->apolloProg = apolloProg;
  traceExit("apollo::code_bone_manager::code_bone_manager()");
}

ScopTypeStruct::ScopTypeStruct(std::shared_ptr<codebones::BoneLoop> &Loop_,
                               osl_scop_t *Verification_,
                               osl_scop_t *Computation_) {
  SupportInspectorExecutor=false;
  Loop = Loop_;
  Verification = Verification_;
  Computation = Computation_;
}

ScopTypeStruct::~ScopTypeStruct() {
  osl_scop_free(Verification);
  osl_scop_free(Computation);
}

void ScopTypeStruct::dump(std::ostream &OS, std::string prefix) {

  // Verif scop
  char *BufVerif = nullptr;
  size_t Size = 0;
  FILE *FPVerif = open_memstream(&BufVerif, &Size);
  OS << prefix << " - Verification scop " << std::endl;
  osl_scop_print(FPVerif, Verification);
  fflush(FPVerif);
  OS << BufVerif;

  // Verif scop
  char *BufCompute = nullptr;
  FILE *FPCompute = open_memstream(&BufCompute, &Size);
  OS << prefix << " - Computation scop " << std::endl;
  osl_scop_print(FPCompute, Computation);
  fflush(FPCompute);
  OS << BufCompute;

  if (SupportInspectorExecutor)
    OS << "TX: Supports inspector executor!\n";

  fclose(FPVerif);
  free(BufVerif);
  fclose(FPCompute);
  free(BufCompute);
}

/// \breif a pure bone statement has the following properties:
///    - it does not contain any store instructions
///    - all its load instructions are linear or tube (i.e they are in the MemInfo object)
///    - all its load instructions are in a read only array
static std::set<apollo::codebones::BoneStmt *>
getPureOrImpureStatements(codebones::BoneLoop *Loop, ArrayAnalysis &MemInfo,
                          bool Pure) {

  std::set<codebones::BoneStmt *> Stmts;
  for (codebones::BoneStmt *stmt : Loop->stmts()) {
    bool IsPure = stmt->stores().empty();
    auto AllAccesses = std::move(stmt->allAccesses());
    for (auto access_iterator = AllAccesses.begin(), end = AllAccesses.end();
         access_iterator != end && IsPure; ++access_iterator) {
      const int MemId = *access_iterator;
      const bool AccessIsPredicted = MemInfo.MemToArray.count(MemId);
      if (AccessIsPredicted) {
        ArrayAnalysis::ArrayInfo *Array = MemInfo.MemToArray[MemId].first;
        if (!Array->ReadOnly) {
          IsPure = false;
        }
      } else {
        IsPure = false;
      }
    }
    if (IsPure == Pure) {
      Stmts.insert(stmt);
    }
  }
  return Stmts;
}

static std::set<apollo::codebones::BoneStmt *>
getVerificationStmts(codebones::BoneLoop *Loop, ArrayAnalysis &MemInfo) {
  return getPureOrImpureStatements(Loop, MemInfo, true);
}

static std::set<apollo::codebones::BoneStmt *>
getComputationStmts(codebones::BoneLoop *Loop, ArrayAnalysis &MemInfo) {
  return getPureOrImpureStatements(Loop, MemInfo, false);
}

bool apollo::CodeBoneManager::initCodeBonesNest(bool Computation,
                                                bool Verification) {

  // Get all the bones in the form of BoneStmt being also BoneAst
  // This form is required to be able to insert theses statements later
  // in the BoneAst representing the loop nest
  traceEnter("apollo::code_bone_manager::init_code_bones_nest()");
  auto CodeBoneDescriptors = std::move(CodeGen.getCodeBonesDescriptors());
  std::vector<std::unique_ptr<BoneStmt>> BoneStmts;
  for (unsigned i = 0; i < CodeBoneDescriptors.size(); ++i) {
    std::unique_ptr<BoneStmt> AStmt(new BoneStmt(CodeBoneDescriptors[i]));
    bool ProducesOutput = !AStmt->stores().empty() || AStmt->isVerification();
    if (ProducesOutput)
      BoneStmts.emplace_back(std::move(AStmt));
  }
  

  // If we have some bones
  if (!BoneStmts.empty()) {

	// Creates the outermost loop representing the initial nest
    this->OutermostLoop = std::move(BoneLoop::createLoopTree(apolloProg));
    BoneLoop *Outermost = this->OutermostLoop.get();

    // insert all the computation stmts (==bones)
    if (Computation) {
      for (unsigned i = 0; i < BoneStmts.size(); ++i) {
        if (BoneStmts[i] && !BoneStmts[i]->stores().empty())
        	Outermost->insertStmtBone(std::move(BoneStmts[i]),
                                   apolloProg);
      }
    }

    // insert all the verification stmts
    if (Verification) {

      // Insert first all verif bones that are not
      // memory verification and tube
      for (unsigned i = 0; i < BoneStmts.size(); ++i) {
        if (BoneStmts[i] && BoneStmts[i]->isVerification()) {
          bool IsTube = false;
          const int MemId = BoneStmts[i]->memVerif().empty()
                            ? -1
                            : BoneStmts[i]->memVerif().front();
          if (MemId != -1) {
            auto MemPred = apolloProg->PredictionModel->getMemPrediction(MemId);
            IsTube = PredictionModel::isTube(MemPred.PreType);
          }
          if (!IsTube) {
        	  Outermost->insertStmtBone(std::move(BoneStmts[i]),
                                     apolloProg);
          }
        }
      }

      // Insert then all memory tube verif bones
      for (unsigned i = 0; i < BoneStmts.size(); ++i) {
        if (BoneStmts[i] && BoneStmts[i]->isVerification()) {
          bool IsTube = false;
          const int MemId = BoneStmts[i]->memVerif().empty()
                            ? -1
                            : BoneStmts[i]->memVerif().front();
          if (MemId != -1) {
            auto MemPred = apolloProg->PredictionModel->getMemPrediction(MemId);
            IsTube = PredictionModel::isTube(MemPred.PreType);
          }
          if (IsTube) {
        	  Outermost->insertStmtBoneTube(std::move(BoneStmts[i]),
                                         apolloProg);
          }
        }
      }
    }
  }
  const bool EmptyBoneNest = !this->OutermostLoop || 
                             this->OutermostLoop.get()->stmts().empty();
  traceExit("apollo::code_bone_manager::init_code_bones_nest()");
  return !EmptyBoneNest;
}

static void attachCandlDependenciesToScop(osl_scop_p &ScopForCandl) {
  traceEnter("attach_candl_dependencies_to_scop()");
  eventStart("code_bones_depanalysis");
  candl_options_p CandlOpt = candl_options_malloc();
  CandlOpt->verbose = false;
  CandlOpt->waw = CandlOpt->raw = CandlOpt->war = true;
  CandlOpt->rar =
      backdoorutils::getEnvBool("APOLLO_BONES", "--rar") ? true : false;
  candl_scop_usr_init(ScopForCandl);
  //codeGenMutex.lock();
  //pthread_mutex_lock(&codeGenMutex);
  //lockMutex();
  osl_dependence_p CandlDeps = candl_dependence(ScopForCandl, CandlOpt);
  //unlockMutex();
  //codeGenMutex.unlock();
  //pthread_mutex_unlock(&codeGenMutex);
  if (CandlDeps) {
    osl_interface_p DepInterface = osl_dependence_interface();
    osl_generic_p Data = osl_generic_shell(CandlDeps, DepInterface);
    osl_generic_add(&ScopForCandl->extension, Data);
  }
  candl_scop_usr_cleanup(ScopForCandl);
  candl_options_free(CandlOpt);
  eventEnd("code_bones_depanalysis");
  traceExit("attach_candl_dependencies_to_scop()");
}

static bool accessGeneratedFromTube(osl_relation_p &Acc) {
  for (int i = 0; i < Acc->nb_rows; ++i) {
    if (!osl_int_zero(OSL_PRECISION, Acc->m[i][0]))
      return true;
  }
  return false;
}

static osl_scop_p fixCandlScopForPluto(osl_scop_p &Scop) {
  traceEnter("fix_candl_scop_for_pluto()");
  for (osl_statement_p stmt = Scop->statement; stmt; stmt = stmt->next) {
    for (osl_relation_list_p acc_list = stmt->access; acc_list;
         acc_list = acc_list->next) {
      osl_relation_p Access = acc_list->elt;
      if (accessGeneratedFromTube(Access)) {
        assert(Access->nb_rows == 3);
        osl_relation_p NewAccess =
            osl_relation_pmalloc(Access->precision, 2, Access->nb_columns);
        osl_relation_set_attributes(
            NewAccess, Access->nb_output_dims, Access->nb_input_dims,
            Access->nb_local_dims, Access->nb_parameters);
        NewAccess->type = Access->type;
        for (int i = 0; i < 2; ++i) {
          osl_int_set_si(OSL_PRECISION, &NewAccess->m[i][0], 0);
          for (int j = 1; j < Access->nb_columns; ++j) {
            int v = osl_int_get_si(OSL_PRECISION, Access->m[i][j]);
            osl_int_set_si(OSL_PRECISION, &NewAccess->m[i][j], v);
          }
        }
        acc_list->elt = NewAccess;
        osl_relation_free(Access);
      }
    }
  }
  traceExit("fix_candl_scop_for_pluto()");
  return Scop;
}

apollo::ScopPtrTy apollo::CodeBoneManager::getScop() {
  traceEnter("apollo::code_bone_manager::get_scop()");
  
  if (!initCodeBonesNest(true, true)) {
    traceExit("code_bone_manager::get_scop empty bone nest");
    return nullptr;
  }

  eventStart("code_bones_depcheck");
  //codeGenMutex.lock();
  bool valid_dependencies =
      codeBonesNestValidDependencies(*this->apolloProg->AA);
  //codeGenMutex.unlock();
  eventEnd("code_bones_depcheck");
  if (!valid_dependencies) {
    traceExit("apollo::code_bone_manager::get_scop dependencies violated");
    return nullptr;
  }

  if (backdoorutils::getEnvBool("APOLLO_BONES", "--simplify"))
    simplifyBoneNest(this->OutermostLoop.get());

  if (backdoorEnabled("info")) {
    std::stringstream OS;
    codebones::BoneAstDump V;
    OS << "code_bone_manager::get_scop reconstructed bone nest [\n";
    V.visit(this->OutermostLoop.get(), OS);
    OS << "]\n";
    Info(OS.str());
  }

  // Extract pure statements from impure ones to be able
  // to optimize verification independently from computation
  eventStart("code_bones_openscop");
  std::set<apollo::codebones::BoneStmt *> Verification =
      std::move(getVerificationStmts(this->OutermostLoop.get(),
                                     *this->apolloProg->AA));
  std::set<apollo::codebones::BoneStmt *> Computation =
      std::move(getComputationStmts(this->OutermostLoop.get(),
                                    *this->apolloProg->AA));
  std::vector<std::set<apollo::codebones::BoneStmt *>> ComputationsPerScop = {
      Verification, Computation};

  if (backdoorEnabled("info")) {
     std::stringstream ToDump;
     ToDump << "Pure statements [\n";
     for (auto stmt : Verification) {
    	 stmt->dump(ToDump);
    	 ToDump << "\n";
     }
     ToDump << "]";
     Info(ToDump.str());
     std::stringstream ToDumpCompute;
     ToDumpCompute << "Impure statements [\n";
     for (auto stmt : Computation) {
    	 stmt->dump(ToDumpCompute);
    	 ToDumpCompute << "\n";
     }
     ToDumpCompute << "]";
     Info(ToDumpCompute.str());
  }

  if (backdoorutils::getEnvBool("APOLLO_BONES", "--no-split")) {
    ComputationsPerScop[1].insert(ComputationsPerScop[0].begin(),
                                  ComputationsPerScop[0].end());
    ComputationsPerScop[0].clear();
  }

  // HACK: I have two sets of scops, one only with linear accesses
  // (for tube, I'll use the regression line)
  // and one with the regression inequalities. Pluto only accepts the first,
  // while dependence analysis requires the second.
  // the results of the dep analysis are embedded in the first to trick Pluto.
  // If we do not do this, Pluto wouldn't be able to choose a tx for the tube.
  std::vector<osl_scop_p> ScopsForPlutoVec = {nullptr, nullptr};
  for (size_t i = 0; i < ComputationsPerScop.size(); ++i) {
    osl_scop_p ScopForCandl = osl_scop_malloc();
    char *Language = (char *)malloc(sizeof(char) * 2);

    Language[0] = 'C';
    Language[1] = 0;

    ScopForCandl->language = Language;
    ScopForCandl->version = 1;
    ScopForCandl->context = buildContext(apolloProg);

    // Compute domains for statements
    BoneToDomain DomainBuilder(this->apolloProg, *this->apolloProg->AA,
                               ComputationsPerScop[i], true);
    BoneToDomain::RetTy Domains =
        DomainBuilder.visit(this->OutermostLoop.get(), 0);

    // Compute scatters for statements
    BoneToScattering ScatterBuilder(this->apolloProg, ComputationsPerScop[i]);
    BoneToScattering::RetTy Scatterings =
        ScatterBuilder.visit(this->OutermostLoop.get(), {0});

    // Compute accesses for statements
    BoneToAccess AccessBuilder(this->apolloProg, *this->apolloProg->AA,
                               ComputationsPerScop[i]);
    BoneToAccess::RetTy Accesses =
        AccessBuilder.visit(this->OutermostLoop.get(), 0);

    // Compute bodies for statements
    BoneToBody BodyBuilder(this->apolloProg, ComputationsPerScop[i]);
    BoneToBody::RetTy Body = BodyBuilder.visit(this->OutermostLoop.get(), 0);

    // Compute arrays for global Extensions
    osl_generic_p Arrays = AccessBuilder.buildArrays();
    osl_generic_add(&ScopForCandl->extension, Arrays);

    // Add statements in the SCoP
    for (auto intr_stmt : this->OutermostLoop->stmts()) {
      if (ComputationsPerScop[i].count(intr_stmt)) {
        osl_statement_p Stmt = osl_statement_malloc();
        Stmt->domain = Domains[intr_stmt];
        Stmt->scattering = Scatterings[intr_stmt];
        Stmt->access = Accesses[intr_stmt];
        Stmt->extension = Body[intr_stmt];
        osl_statement_add(&ScopForCandl->statement, Stmt);
      }
    }

    // Add parameters
    osl_strings_p ParametersNames = osl_strings_malloc();
    free(ParametersNames->string);
    ParametersNames->string = (char **)malloc(sizeof(char *) * 3);
    ParametersNames->string[0] = strdup("chunk_lower");
    ParametersNames->string[1] = strdup("chunk_upper");
    ParametersNames->string[2] = 0;
    osl_generic_p Parameters = osl_generic_malloc();
    Parameters->interface = osl_strings_interface();
    Parameters->data = ParametersNames;
    osl_generic_add(&ScopForCandl->parameters, Parameters);

    //codeGenMutex.lock();
    // Now do the dependence analysis and attach it to the Scop
    attachCandlDependenciesToScop(ScopForCandl);
    //codeGenMutex.unlock();
    // Fix the scop to make it compatible with Pluto
    // replace tubes by regression lines in the original
    // accesses function. This is required even if we modified
    // pluto to use dependencies computed by Candl instead
    // of computing dependencies itself using either ISL or Candl.
    // Indeed, pluto still uses the access functions in some phases
    // and crashes if they are not pure linear functions.
    osl_scop_p ScopForPluto = fixCandlScopForPluto(ScopForCandl);
    ScopsForPlutoVec[i] = ScopForPluto;
    
  }
  
  apollo::ScopPtrTy ScopPtr = std::make_shared<ScopTypeStruct>(
      this->OutermostLoop, ScopsForPlutoVec[0], ScopsForPlutoVec[1]);

  // If all the verification bones have been extracted in the pure
  // scop, then we support inspector/executor execution
  ScopPtr->SupportInspectorExecutor = true;
  for (apollo::codebones::BoneStmt *bone : ComputationsPerScop[1]) {
    if (bone->isVerification()) {
      ScopPtr->SupportInspectorExecutor = false;
      break;
    }
  }
  eventEnd("code_bones_openscop");

  if (backdoorEnabled("info")) {
    std::stringstream ToDump;
    ScopPtr->dump(ToDump, "Before optimization");
    Info(ToDump.str());
  }
  
  traceExit("apollo::code_bone_manager::get_scop success");
  return ScopPtr;
}

static PlutoOptions *getPlutoOptions(bool IsComputation, bool Tile = false) {
  int Fuse = SMART_FUSE;
  bool Innerpar = false;
  bool UseIsl = false;
  bool Identity = false;
  bool Unroll = false;
  bool Parallel = true;
  bool Debug = false;
  //Raquel New
  bool Intratileopt = true;
  bool L2tile = false;
  bool Partlbtile = false;
  bool Iss = true;
  bool Lbtile = false;
  bool Rar = false;
  bool Forceparallel = false;
  bool Multipipe = false;
  bool Polyunroll = false;
  bool Time = false;

  //end Raquel New

  // get options from the environment.
  Tile =
      backdoorutils::getEnvBool("APOLLO_BONES", "--notile")
          ? false
          : (backdoorutils::getEnvBool("APOLLO_BONES", "--tile") ? true : Tile);
  Innerpar =
      backdoorutils::getEnvBool("APOLLO_BONES", "--innerpar") ? true : Innerpar;
  UseIsl =
      backdoorutils::getEnvBool("APOLLO_BONES", "--islsolve") ? true : UseIsl;
  Identity =
      backdoorutils::getEnvBool("APOLLO_BONES", "--identity") ? true : Identity;
  Unroll =
      backdoorutils::getEnvBool("APOLLO_BONES", "--unroll") ? true : Unroll;

  std::string FuseOption = backdoorutils::getEnv("APOLLO_BONES", "--fuse");
  if (FuseOption == "no")
    Fuse = NO_FUSE;
  else if (FuseOption == "smart")
    Fuse = SMART_FUSE;
  else if (FuseOption == "max")
    Fuse = MAXIMAL_FUSE;

  Parallel = backdoorutils::getEnvBool("APOLLO_BONES", "--noparallel")
                 ? false
                 : Parallel;
  Debug = backdoorutils::getEnvBool("APOLLO_BONES", "--debug") ? true : Debug;

// Raquel New
  Intratileopt =
      backdoorutils::getEnvBool("APOLLO_BONES", "--nointratileopt")
          ? false
          : (backdoorutils::getEnvBool("APOLLO_BONES", "--intratileopt") ? true : Intratileopt);
  L2tile =
      backdoorutils::getEnvBool("APOLLO_BONES", "--l2tile") ? true : L2tile;
  Partlbtile =
      backdoorutils::getEnvBool("APOLLO_BONES", "--partlbtile") ? true : Partlbtile;
  Iss =
      backdoorutils::getEnvBool("APOLLO_BONES", "--noiss") ? false : Iss;
  Lbtile =
      backdoorutils::getEnvBool("APOLLO_BONES", "--lbtile") ? true : Lbtile;
  Rar =
      backdoorutils::getEnvBool("APOLLO_BONES", "--rar") ? true : Rar;
  Forceparallel =
      backdoorutils::getEnvBool("APOLLO_BONES", "--forceparallel") ? true : Forceparallel;
  Multipipe =
      backdoorutils::getEnvBool("APOLLO_BONES", "--multipipe") ? true : Multipipe;
  Polyunroll =
      backdoorutils::getEnvBool("APOLLO_BONES", "--polyunroll") ? true : Polyunroll;
  Time =
      backdoorutils::getEnvBool("APOLLO_BONES", "--time") ? true : Time;
// end Raquel New  

  if (!IsComputation) {
    Tile = false;
    Fuse = NO_FUSE;
    Identity = false;
  }

  PlutoOptions *Options = pluto_options_alloc();
  Options->quiet = true;
  Options->debug = Debug;
  Options->moredebug = false;
  Options->silent = true;
  Options->prevector = Tile;
  Options->tile = Tile;
  //Options->intratileopt = true;
  Options->intratileopt = Intratileopt; //Raquel New
  //Options->l2tile = false;
  Options->l2tile = L2tile; //Raquel New
  Options->parallel = Parallel;
  Options->fuse = Fuse; // SMART_FUSE, MAXIMAL_FUSE, NO_FUSE
  Options->unroll = Unroll;
  Options->ufactor = 2;
  //Options->iss = true;
  Options->iss = Iss; // Raquel New
  Options->innerpar = Innerpar;
  Options->candldep = !UseIsl;
  Options->isldep = UseIsl;
  Options->islsolve = UseIsl;
  Options->identity = Identity;
  // Raquel New 
  Options->lbtile = Lbtile;
  Options->partlbtile = Partlbtile;
  Options->rar = Rar;
  Options->forceparallel = Forceparallel;
  Options->multipipe = Multipipe;
  Options->polyunroll = Polyunroll;
  Options->time = Time;
  //end Raquel New
  return Options;
}

apollo::ScopPtrTy apollo::CodeBoneManager::getOptimized(ScopPtrTy &Original, 
  TxInfo &InfoTx, std::string uniqueId) {

  traceEnter("apollo::code_bone_manager::get_optimized");
  if (!Original) {
    traceExit("apollo::code_bone_manager::get_optimized");
    return nullptr;
  }

  eventStart("code_bones_polyhedral_scheduler");
  bool Tile = shouldTile(apolloProg->InstrumentationResults,
                         apolloProg->PredictionModel,
                         apolloProg->StaticInfo.get(), Original->Loop);

  PlutoOptions *VerifOptions;
  PlutoOptions *CompOptions;
  int dim = 32;
  std::vector<int> tiles;
  bool tile = false;
  if(!apolloRuntimeConfig.multiVersioning){
    VerifOptions = getPlutoOptions(false);
    CompOptions = getPlutoOptions(true, Tile);
  }else{
    /*auto it = apolloRuntimeConfig.allPlutoOptsTried.find(uniqueId);
    if(it != apolloRuntimeConfig.allPlutoOptsTried.end()){
      CompOptions = apolloRuntimeConfig.PlutoOptsSet.at(apolloRuntimeConfig.allPlutoOptsTried.at(uniqueId).size()).first;
      //CompOptions = apolloRuntimeConfig.PlutoOptsSet.at(apolloRuntimeConfig.allPlutoOptsTried.at(uniqueId).size()).first;
      tile = CompOptions->tile;
      tileSize = apolloRuntimeConfig.PlutoOptsSet.at(apolloRuntimeConfig.allPlutoOptsTried.at(uniqueId).size()).second.first;
      tileType = apolloRuntimeConfig.PlutoOptsSet.at(apolloRuntimeConfig.allPlutoOptsTried.at(uniqueId).size()).second.second;
      //printf("tile = %d, tile Size = %d, homogeneous tile = %d\n", tile, tileSize, homogeneousTile);
    }else{*/
      CompOptions = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed()).first;
      tile = CompOptions->tile;
      dim = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed()).second.size();
      tiles = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed()).second;
      std::string opts = InfoTx.plutoOptionsToString(CompOptions, tiles);
      if(InfoTx.txAlreadyAnalyzed(opts)){
        CompOptions = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed() + 1).first;
        tile = CompOptions->tile;
        dim = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed() + 1).second.size();
        tiles = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed() + 1).second;
      }
      
      //printf("tile = %d, tile Size = %d, homogeneous tile = %d\n", tile, tileSize, homogeneousTile);
    //}

    //CompOptions = apolloRuntimeConfig.PlutoOptsSet.at(InfoTx.getSizePlutoOptsAnalyzed());
    VerifOptions = getPlutoOptions(false);
  }

  apollo::ScopPtrTy Transformed = std::make_shared<ScopTypeStruct>(
      Original->Loop, osl_scop_clone(Original->Verification),
      osl_scop_clone(Original->Computation));

  if(apolloRuntimeConfig.multiVersioning){
    InfoTx.storePlutoOptions(CompOptions, tiles);
  }

  const double plutoInit = eventStartTxMeasure("plutoTimeStart,");
  apolloRuntimeConfigMutex.lock();
  if(apolloRuntimeConfig.multiVersioning && tile){
    FILE *tsfile = fopen("tile.sizes", "w");
    int d = apolloProg->StaticInfo->MaxLoopDepth;

    if (dim < d)
      printf("Completed value from %d to %d with 1\n", dim, d);

    // complete missing tile with value 1
    for(int i=dim; i<=d; ++i)
      tiles.push_back(1);
    
    // save all tile sizes in tile.sizes
    for(auto it=tiles.begin(); it!=tiles.begin()+d; ++it)
      fprintf(tsfile, "%d\n", *it);

    fclose(tsfile);
  }

  //End Raquel New
  pluto_schedule_osl(Transformed->Verification, VerifOptions);
  pluto_schedule_osl(Transformed->Computation, CompOptions);
  apolloRuntimeConfigMutex.unlock();
  //Raquel New
  const double plutoEnd = eventEndTxMeasure("plutoTimeEnd,");
  InfoTx.setPlutoTime((plutoEnd-plutoInit));
  //End Raquel New

  if(!apolloRuntimeConfig.multiVersioning){
    pluto_options_free(CompOptions);
    pluto_options_free(VerifOptions);
  }

  Transformed->SupportInspectorExecutor = Original->SupportInspectorExecutor;

  if (backdoorEnabled("info")) {
     std::stringstream ToDump;
     Transformed->dump(ToDump, "After optimization");
     Info(ToDump.str());
   }

  eventEnd("code_bones_polyhedral_scheduler");
  traceExit("apollo::code_bone_manager::get_optimized");
  return Transformed;
}

ScanTypeStruct::ScanTypeStruct(int F) {
  SupportInspectorExecutor=false;
  State = cloog_state_malloc();
  Options = cloog_options_malloc(State);
  Options->quiet = true;
  Options->strides = false;
  Options->sh = false;
  Options->nosimplify = false;

  if (!backdoorutils::getEnvBool("APOLLO_BONES", "--control"))
    Options->f = F;

  Options->save_domains = false;
  Options->noscalars = true;
  ClastVerification = nullptr;
  ClastComputation = nullptr;
}

ScanTypeStruct::~ScanTypeStruct() {
  if (ClastVerification)
    cloog_clast_free(ClastVerification);
  if (ClastComputation)
    cloog_clast_free(ClastComputation);
  cloog_options_free(Options);
  cloog_state_free(State);
}

void ScanTypeStruct::dump(std::ostream &OS) {
  if (ClastVerification && ClastComputation) {
    char *Buf = nullptr;
    size_t Size = 0;
    FILE *Fp = open_memstream(&Buf, &Size);
    fprintf(Fp, "//independent verification.\n\n");
    clast_pprint(Fp, ClastVerification, 0, Options);
    fprintf(Fp, "\n//computation.\n\n");
    clast_pprint(Fp, ClastComputation, 0, Options);
    fprintf(Fp, "\n");

    if (SupportInspectorExecutor)
      fprintf(Fp, "\tsupports inspector executor.\n");

    const std::string names[] = {"verification", "computation"};
    // do not print for verification
    for (int id = 1; id < 2; ++id) {
      fprintf(Fp, "\t%s.parallel = [ ", names[id].c_str());
      for (const auto &iter : Parallel[id]) {
        fprintf(Fp, "%s { ", iter.first.c_str());
        for (auto stmt_id : iter.second) {
          fprintf(Fp, "%d ", stmt_id);
        }
        fprintf(Fp, "} ");
      }
      fprintf(Fp, "].\n\t%s.vectorize = [", names[id].c_str());
      for (const auto &iter : Vectorize[id]) {
        fprintf(Fp, "%s { ", iter.first.c_str());
        for (auto stmt_id : iter.second) {
          fprintf(Fp, "%d ", stmt_id);
        }
        fprintf(Fp, "} ");
      }
      fprintf(Fp, "].\n\t%s.unroll = [", names[id].c_str());
      for (const auto &iter : Unroll[id]) {
        fprintf(Fp, "%s by %d ", iter.first.c_str(), iter.second.second);
      }
      fprintf(Fp, "].\n");
    }
    fflush(Fp);
    OS << Buf;
    fclose(Fp);
    free(Buf);
  }
}

static void markParallel(osl_scop_p &Scop, apollo::ScanPtrTy &Scan, int Idx) {
  osl_loop_p Loop =
      (osl_loop_p)osl_generic_lookup(Scop->extension, OSL_URI_LOOP);

  while (Loop) {
    int Directive = Loop->directive;
    bool Vectorize = Directive == OSL_LOOP_DIRECTIVE_VECTOR;
    bool Parallel = Directive == OSL_LOOP_DIRECTIVE_PARALLEL;
    if (Vectorize || Parallel) {
      std::pair<std::string, std::set<int>> IteratorStmtsPair;
      IteratorStmtsPair.first = Loop->iter;
      for (int i = 0; i < Loop->nb_stmts; ++i) {
        IteratorStmtsPair.second.insert(Loop->stmt_ids[i]);
      }
      if (Vectorize)
        Scan->Vectorize[Idx].insert(IteratorStmtsPair);
      else if (Parallel)
        Scan->Parallel[Idx].insert(IteratorStmtsPair);
    }
    Loop = Loop->next;
  }
}

static void markUnroll(osl_scop_p &Scop, apollo::ScanPtrTy &Scan, int Idx) {
  osl_pluto_unroll_p Unroll = (osl_pluto_unroll_p)osl_generic_lookup(
      Scop->extension, OSL_URI_PLUTO_UNROLL);
  while (Unroll) {
    Scan->Unroll[Idx][Unroll->iter] =
        std::pair<bool, int>(Unroll->jam, Unroll->factor);
    Unroll = Unroll->next;
  }
}

static void mapStmtWithNames(osl_scop_p &Scop, ScanPtrTy &Scan, int Idx) {
  osl_statement_p Stmt = Scop->statement;
  for (int stmt_id = 1; Stmt; Stmt = Stmt->next, ++stmt_id) {
    osl_body_p StmtBody =
        (osl_body_p)osl_generic_lookup(Stmt->extension, OSL_URI_BODY);
    assert(StmtBody);
    std::string StmtName = StmtBody->expression->string[0];
    Scan->StmtIdToName[Idx][stmt_id] = StmtName;
  }
}

apollo::ScanPtrTy apollo::CodeBoneManager::getScan(ScopPtrTy &Scop) {
  traceEnter("apollo::code_bone_manager::get_scan");
  if (!Scop) {
    traceExit("apollo::code_bone_manager::get_scan");
    return nullptr;
  }

  eventStart("code_bones_scan");
  apollo::ScanPtrTy Scan = std::make_shared<ScanTypeStruct>();
  Scan->Loop = Scop->Loop;

  CloogInput *VerificationInput =
      cloog_input_from_osl_scop(Scan->State, Scop->Verification);
  Scan->ClastVerification =
      cloog_clast_create_from_input(VerificationInput, Scan->Options);

  CloogInput *ComputationInput =
      cloog_input_from_osl_scop(Scan->State, Scop->Computation);
  Scan->ClastComputation =
      cloog_clast_create_from_input(ComputationInput, Scan->Options);

  Scan->SupportInspectorExecutor = Scop->SupportInspectorExecutor;

  markParallel(Scop->Verification, Scan, 0);
  markUnroll(Scop->Verification, Scan, 0);
  mapStmtWithNames(Scop->Verification, Scan, 0);

  markParallel(Scop->Computation, Scan, 1);
  markUnroll(Scop->Computation, Scan, 1);
  mapStmtWithNames(Scop->Computation, Scan, 1);

  eventEnd("code_bones_scan");

  if (backdoorEnabled("info")) {
    std::stringstream ToDump;
    Scan->dump(ToDump);
    Info(ToDump.str());
  }
  traceExit("apollo::code_bone_manager::get_scan");
  return Scan;
}
