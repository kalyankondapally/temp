/*
// Copyright (c) 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "hwcutils.h"
#include "hwctrace.h"
#include "option.h"
#include "optionmanager.h"
#include "platformdefines.h"

namespace hwcomposer {

#if INTEL_HWC_INTERNAL_BUILD
#define WANT_PARTIAL_MATCH 1
#else
#define WANT_PARTIAL_MATCH 0
#endif

void OptionManager::add(Option* pOption) {
  HWC_ASSERT_LOCK_NOT_HELD(mLock);
  ScopedSpinLock _l(mLock);
  mOptions.push_back(pOption);
}

void OptionManager::remove(Option* pOption) {
  HWC_ASSERT_LOCK_NOT_HELD(mLock);
  ScopedSpinLock _l(mLock);
  for (int32_t i = mOptions.size() - 1; i >= 0; i--) {
    if (mOptions[i] == pOption) {
      // FIXME(Kalyan)
      // mOptions.removeAt(i);
    }
  }
}

HWCString OptionManager::dump() {
  HWC_ASSERT_LOCK_NOT_HELD(mLock);
  ScopedSpinLock _l(mLock);
  HWCString output("Option Values:");

  for (uint32_t i = 0; i < mOptions.size(); i++) {
    if (mOptions[i]->isInitialized()) {
      output += "\n";
      output += mOptions[i]->dump();
    }
  }

  return output;
}

Option* OptionManager::find(const char* optionName, bool bExact) {
  return OptionManager::getInstance().findInternal(optionName, bExact);
}

Option* OptionManager::findInternal(const char* optionName, bool bExact) {
  HWC_ASSERT_LOCK_NOT_HELD(mLock);

  HWCString sOption(optionName);

  // Compare in lower case only.
  sOption.toLower();

  // Compare exactly to both supplied and prefixed.
  HWCString sPrefixedOption(Option::getPropertyRoot());
  sPrefixedOption += sOption;

  // Find candidates and exact matches.
  uint32_t exact = mOptions.size();
  uint32_t exactAlternate = mOptions.size();

#if WANT_PARTIAL_MATCH
  uint32_t matches = 0;
  HWCString matchString;
  uint32_t candidate = mOptions.size();
  uint32_t candidateAlternate = mOptions.size();
  uint32_t matchesAlternate = 0;
  HWCString matchStringAlternate;
#else
  HWC_UNUSED(bExact);
#endif

  ScopedSpinLock _l(mLock);

  bool bEmpty = (sOption.length() == 0);

  for (uint32_t opt = 0; opt < mOptions.size(); ++opt) {
    const Option& option = *mOptions[opt];
    if (option.getPropertyString().length() == 0)
      continue;

    // Exactly matched masters.
    if (!bEmpty && ((option.getPropertyString() == sOption) ||
                    (option.getPropertyString() == sPrefixedOption))) {
      if (exact < mOptions.size()) {
        ETRACE("Option '%s' exactly matches %s and %s", sOption.string(),
               mOptions[exact]->getPropertyString().string(),
               option.getPropertyString().string());
      } else {
        exact = opt;
      }
    }

    // Exactly matched alternates.
    if (!bEmpty && ((option.getPropertyStringAlternate() == sOption) ||
                    (option.getPropertyStringAlternate() == sPrefixedOption))) {
      if (exactAlternate < mOptions.size()) {
        ETRACE("Option '%s' exactly matches alternate %s and %s",
               sOption.string(),
               mOptions[exactAlternate]->getPropertyStringAlternate().string(),
               option.getPropertyStringAlternate().string());
      } else {
        exactAlternate = opt;
      }
    }
#if WANT_PARTIAL_MATCH
    // Partially matched master candidates.
    else if (!bExact) {
      if (bEmpty || (option.getPropertyString().find(sOption) != -1)) {
        matchString += HWCString("\n  ") + option.getPropertyString();
        candidate = opt;
        ++matches;
      } else if (option.getPropertyStringAlternate().find(sOption) != -1) {
        matchStringAlternate +=
            HWCString("\n  ") + option.getPropertyStringAlternate();
        candidateAlternate = opt;
        ++matchesAlternate;
      }
    }
#endif
  }

  // Prioritize matches:
  // 1/ exact matches.
  if (exact < mOptions.size()) {
    ITRACE("Matching option %s", mOptions[exact]->getPropertyString().string());
    if (mOptions[exact]->isPermitChange()) {
      // FIXME
      // return mOptions.editItemAt(exact);
    }
    ETRACE("Matching option %s immutable",
           mOptions[exact]->getPropertyString().string());
    return NULL;
  }
  // 2/ exact matches on alternate strings.
  else if (exactAlternate < mOptions.size()) {
    ITRACE("Matching option %s  (from alternate:%s)",
           mOptions[exactAlternate]->getPropertyString().string(),
           mOptions[exactAlternate]->getPropertyStringAlternate().string());
    if (mOptions[exactAlternate]->isPermitChange()) {
      // FIXME
      // return mOptions.editItemAt(exactAlternate);
    }
    ETRACE("Matching option %s immutable",
           mOptions[exactAlternate]->getPropertyString().string());
    return NULL;
  }
#if WANT_PARTIAL_MATCH
  // 3/ partial matches on strings.
  else if (matches > 1) {
    ETRACE("Option '%s' matches %u options: %s", sOption.string(), matches,
           matchString.string());
    return NULL;
  } else if (candidate < mOptions.size()) {
    ITRACE("Matching option %s",
           mOptions[candidate]->getPropertyString().string());
    if (mOptions[candidate]->isPermitChange()) {
      return mOptions.editItemAt(candidate);
    }
    ETRACE("Matching option %s immutable",
           mOptions[candidate]->getPropertyString().string());
    return NULL;
  }
  // 4/ partial matches on alternate strings.
  else if (matchesAlternate > 1) {
    ETRACE("Option '%s' matches %u alternate options: %s", sOption.string(),
           matchesAlternate, matchStringAlternate.string());
    return NULL;
  } else if (candidateAlternate < mOptions.size()) {
    ITRACE("Matching option %s (from alternate:%s)",
           mOptions[candidateAlternate]->getPropertyString().string(),
           mOptions[candidateAlternate]->getPropertyStringAlternate().string());
    if (mOptions[candidateAlternate]->isPermitChange()) {
      return mOptions.editItemAt(candidateAlternate);
    }
    ETRACE("Matching option %s immutable",
           mOptions[candidateAlternate]->getPropertyString().string());
    return NULL;
  }
#endif
  ETRACE("Option '%s' not recognised.", sOption.string());

  return NULL;
}

};  // namespace hwcomposer
