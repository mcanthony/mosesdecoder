// $Id$
// vim:tabstop=2
/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#include <fstream>
#include <string>
#include <iterator>
#include <queue>
#include <algorithm>
#include <sys/stat.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread/tss.hpp>

#include "PhraseDictionaryCompact.h"
#include "moses/FactorCollection.h"
#include "moses/Word.h"
#include "moses/Util.h"
#include "moses/InputFileStream.h"
#include "moses/StaticData.h"
#include "moses/Range.h"
#include "moses/ThreadPool.h"
#include "util/exception.hh"

using namespace std;
using namespace boost::algorithm;

namespace Moses
{

PhraseDictionaryCompact::SentenceCache PhraseDictionaryCompact::m_sentenceCache;

PhraseDictionaryCompact::PhraseDictionaryCompact(const std::string &line)
  :PhraseDictionary(line, true)
  ,m_inMemory(true)
  ,m_useAlignmentInfo(true)
  ,m_hash(10, 16)
  ,m_phraseDecoder(0)
  ,m_weight(0)
{
  ReadParameters();
}

void PhraseDictionaryCompact::Load()
{
  const StaticData &staticData = StaticData::Instance();

  SetFeaturesToApply();

  m_weight = staticData.GetWeights(this);

  std::string tFilePath = m_filePath;

  std::string suffix = ".minphr";
  if (!ends_with(tFilePath, suffix)) tFilePath += suffix;
  if (!FileExists(tFilePath))
    throw runtime_error("Error: File " + tFilePath + " does not exist.");

  m_phraseDecoder = new PhraseDecoder(*this, &m_input, &m_output,
                                      m_numScoreComponents, &m_weight);

  std::FILE* pFile = std::fopen(tFilePath.c_str() , "r");

  size_t indexSize;
  //if(m_inMemory)
  // Load source phrase index into memory
  indexSize = m_hash.Load(pFile);
// else
  // Keep source phrase index on disk
  //indexSize = m_hash.LoadIndex(pFile);

  size_t coderSize = m_phraseDecoder->Load(pFile);

  size_t phraseSize;
  if(m_inMemory)
    // Load target phrase collections into memory
    phraseSize = m_targetPhrasesMemory.load(pFile, false);
  else
    // Keep target phrase collections on disk
    phraseSize = m_targetPhrasesMapped.load(pFile, true);

  UTIL_THROW_IF2(indexSize == 0 || coderSize == 0 || phraseSize == 0,
                 "Not successfully loaded");
}

// now properly declared in TargetPhraseCollection.h
// and defined in TargetPhraseCollection.cpp
// struct CompareTargetPhrase {
//   bool operator() (const TargetPhrase &a, const TargetPhrase &b) {
//     return a.GetFutureScore() > b.GetFutureScore();
//   }
// };

TargetPhraseCollection::shared_ptr
PhraseDictionaryCompact::GetTargetPhraseCollectionNonCacheLEGACY(const Phrase &sourcePhrase) const
{

  TargetPhraseCollection::shared_ptr ret;
  // There is no souch source phrase if source phrase is longer than longest
  // observed source phrase during compilation
  if(sourcePhrase.GetSize() > m_phraseDecoder->GetMaxSourcePhraseLength())
    return ret;

  // Retrieve target phrase collection from phrase table
  TargetPhraseVectorPtr decodedPhraseColl
  = m_phraseDecoder->CreateTargetPhraseCollection(sourcePhrase, true, true);

  if(decodedPhraseColl != NULL && decodedPhraseColl->size()) {
    TargetPhraseVectorPtr tpv(new TargetPhraseVector(*decodedPhraseColl));
    TargetPhraseCollection::shared_ptr  phraseColl(new TargetPhraseCollection);

    // Score phrases and if possible apply ttable_limit
    TargetPhraseVector::iterator nth =
      (m_tableLimit == 0 || tpv->size() < m_tableLimit) ?
      tpv->end() : tpv->begin() + m_tableLimit;
    NTH_ELEMENT4(tpv->begin(), nth, tpv->end(), CompareTargetPhrase());
    for(TargetPhraseVector::iterator it = tpv->begin(); it != nth; it++) {
      TargetPhrase *tp = new TargetPhrase(*it);
      phraseColl->Add(tp);
    }

    // Cache phrase pair for clean-up or retrieval with PREnc
    const_cast<PhraseDictionaryCompact*>(this)->CacheForCleanup(phraseColl);

    return phraseColl;
  } else
    return ret;
}

TargetPhraseVectorPtr
PhraseDictionaryCompact::GetTargetPhraseCollectionRaw(const Phrase &sourcePhrase) const
{

  // There is no such source phrase if source phrase is longer than longest
  // observed source phrase during compilation
  if(sourcePhrase.GetSize() > m_phraseDecoder->GetMaxSourcePhraseLength())
    return TargetPhraseVectorPtr();

  // Retrieve target phrase collection from phrase table
  return m_phraseDecoder->CreateTargetPhraseCollection(sourcePhrase, true, false);
}

PhraseDictionaryCompact::~PhraseDictionaryCompact()
{
  if(m_phraseDecoder)
    delete m_phraseDecoder;
}

//TO_STRING_BODY(PhraseDictionaryCompact)

void PhraseDictionaryCompact::CacheForCleanup(TargetPhraseCollection::shared_ptr  tpc)
{
  if(!m_sentenceCache.get())
    m_sentenceCache.reset(new PhraseCache());
  m_sentenceCache->push_back(tpc);
}

void PhraseDictionaryCompact::AddEquivPhrase(const Phrase &source,
    const TargetPhrase &targetPhrase) { }

void PhraseDictionaryCompact::CleanUpAfterSentenceProcessing(const InputType &source)
{
  if(!m_sentenceCache.get())
    m_sentenceCache.reset(new PhraseCache());

  m_phraseDecoder->PruneCache();
  // for(PhraseCache::iterator it = m_sentenceCache->begin();
  //     it != m_sentenceCache->end(); it++)
  //   it->reset();

  // PhraseCache temp;
  // temp.swap(*m_sentenceCache);
  m_sentenceCache->clear();

  ReduceCache();
}

}

