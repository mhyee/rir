#ifndef RIR_RIR_REGISTER_MAP_H
#define RIR_RIR_REGISTER_MAP_H

#include "RirRuntimeObject.h"
#include "runtime/TypeFeedback.h"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace rir {

#pragma pack(push)
#pragma pack(1)

constexpr static size_t PIR_REGISTER_MAP_MAGIC = 0x31573;

struct Code;

struct PirRegisterMap
    : public RirRuntimeObject<PirRegisterMap, PIR_REGISTER_MAP_MAGIC> {
  public:
    constexpr static size_t MAX_SLOT_IDX = 64;

    static PirRegisterMap*
    New(const std::unordered_set<Code*>& origins,
        const std::unordered_map<size_t, std::pair<Code*, Opcode*>>& slots) {
        SEXP cont = Rf_allocVector(EXTERNALSXP,
                                   requiredSize(origins.size(), slots.size()));
        PirRegisterMap* res =
            new (DATAPTR(cont)) PirRegisterMap(origins, slots);
        return res;
    }

    PirRegisterMap(
        const std::unordered_set<Code*>& codes,
        const std::unordered_map<size_t, std::pair<Code*, Opcode*>>& slots);

    ObservedValues& getSampleOfSlot(size_t slot) {
        return getMDEntryOfSlot(slot).feedback;
    }

    Opcode* getOriginOfSlot(size_t slot) {
        return getMDEntryOfSlot(slot).origin;
    }

    static size_t requiredSize(size_t origins, size_t entries) {
        return sizeof(PirRegisterMap) + sizeof(SEXP) * origins +
               sizeof(MDEntry) * entries;
    }

  private:
    struct MDEntry {
        Opcode* origin;
        ObservedValues feedback;
    };
    static_assert(sizeof(MDEntry) == 12, "");

    MDEntry& getMDEntryOfSlot(size_t slot) {
        assert(slot < MAX_SLOT_IDX);
        auto idx = entry[slot];
        assert(idx != MAX_SLOT_IDX);
        return mdEntries()[idx];
    }

    MDEntry* mdEntries() {
        return reinterpret_cast<MDEntry*>((uintptr_t)this + info.gc_area_start +
                                          sizeof(SEXP) * info.gc_area_length);
    }

    uint8_t entry[MAX_SLOT_IDX];
};

#pragma pack(pop)
} // namespace rir

#endif