#include "PirTypeFeedback.h"
#include "Code.h"
#include <unordered_map>

namespace rir {

const size_t PirTypeFeedback::MAX_SLOT_IDX;

PirTypeFeedback::PirTypeFeedback(
    const std::unordered_set<Code*>& codes,
    const std::unordered_map<size_t, std::pair<Code*, Opcode*>>& slots)
    : RirRuntimeObject(sizeof(*this), codes.size()) {
    assert((size_t)XLENGTH(container()) >=
           requiredSize(codes.size(), slots.size()));
    memset(entry, MAX_SLOT_IDX, sizeof(entry));
    assert(slots.size() < MAX_SLOT_IDX);
    assert(codes.size() < MAX_SLOT_IDX);
    static_assert(MAX_SLOT_IDX <= 0xff * sizeof(uint8_t), "");

    // Store all origins in the gc area, to keep them live
    // TODO, is this really needed? or is there any guarantee that my baseline
    // and all inlinee's baseline code objects stay live? also this should
    // probably be a weak map instead...
    std::unordered_map<Code*, uint8_t> srcCodeMap;
    size_t idx = 0;
    for (auto c : codes) {
        srcCodeMap[c] = idx;
        setEntry(idx++, c->container());
    }

    idx = 0;

    std::unordered_map<Opcode*, size_t> reverseMapping;

    for (auto s : slots) {
        auto slot = s.first;
        auto origin = s.second;
        assert(slot < MAX_SLOT_IDX);

        auto e = reverseMapping.find(origin.second);
        if (e != reverseMapping.end()) {
            entry[slot] = e->second;
        } else {
            assert(codes.count(origin.first));
            new (&mdEntries()[idx]) MDEntry;
            mdEntries()[idx].srcCode = srcCodeMap.at(origin.first);
            mdEntries()[idx].offset = origin.second - origin.first->code();
            reverseMapping[origin.second] = idx;
            entry[slot] = idx++;
        }
    }
}

Code* PirTypeFeedback::getSrcCodeOfSlot(size_t slot) {
    auto code = getEntry(getMDEntryOfSlot(slot).srcCode);
    return Code::unpack(code);
}

Opcode* PirTypeFeedback::getOriginOfSlot(size_t slot) {
    return getSrcCodeOfSlot(slot)->code() + getBCOffsetOfSlot(slot);
}

} // namespace rir
