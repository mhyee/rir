//===-- StatepointDefaultGC.cpp - The default statepoint GC strategy ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a GCStrategy which serves as an example for the usage
// of a statepoint based lowering strategy.  This GCStrategy is intended to
// suitable as a default implementation usable with any collector which can
// consume the standard stackmap format generated by statepoints, uses the
// default addrespace to distinguish between gc managed and non-gc managed
// pointers, and has reasonable relocation semantics.
//
//===----------------------------------------------------------------------===//

#include "JITGCStrategy.h"

#include "Types.h"

#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Value.h"

using namespace llvm;

namespace rjit {
JITStatepointGC::JITStatepointGC() {
    UseStatepoints = true;
    // These options are all gc.root specific, we specify them so that the
    // gc.root lowering code doesn't run.
    InitRoots = false;
    NeededSafePoints = 0;
    UsesMetadata = false;
    CustomRoots = false;
}
bool JITStatepointGC::isGCManaged(const Value* V) {
    if (isa<Constant>(V))
        return false;

    if (isa<PHINode>(V) && cast<PHINode>(V)->hasConstantValue())
        return false;

    Type* T = V->getType();
    return !isa<Constant>(V) && isa<PointerType>(T) && T == t::SEXP;
}
Optional<bool> JITStatepointGC::isGCManagedPointer(const Value* V) const {
    // Just a basic sanity check for the backend
    return 1 == V->getType()->getPointerAddressSpace();
}
}

static GCRegistry::Add<rjit::JITStatepointGC> X("rjit",
                                                "rjit strategy for statepoint");
