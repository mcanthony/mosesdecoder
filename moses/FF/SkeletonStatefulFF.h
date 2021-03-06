#pragma once

#include <string>
#include "StatefulFeatureFunction.h"
#include "FFState.h"

namespace Moses
{

class SkeletonState : public FFState
{
  int m_targetLen;
public:
  SkeletonState(int targetLen)
    :m_targetLen(targetLen) {
  }

  virtual size_t hash() const {
    return (size_t) m_targetLen;
  }
  virtual bool operator==(const FFState& o) const {
    const SkeletonState& other = static_cast<const SkeletonState&>(o);
    return m_targetLen == other.m_targetLen;
  }

};

class SkeletonStatefulFF : public StatefulFeatureFunction
{
public:
  SkeletonStatefulFF(const std::string &line);

  bool IsUseable(const FactorMask &mask) const {
    return true;
  }
  virtual const FFState* EmptyHypothesisState(const InputType &input) const {
    return new SkeletonState(0);
  }

  void EvaluateInIsolation(const Phrase &source
                           , const TargetPhrase &targetPhrase
                           , ScoreComponentCollection &scoreBreakdown
                           , ScoreComponentCollection &estimatedScore) const;
  void EvaluateWithSourceContext(const InputType &input
                                 , const InputPath &inputPath
                                 , const TargetPhrase &targetPhrase
                                 , const StackVec *stackVec
                                 , ScoreComponentCollection &scoreBreakdown
                                 , ScoreComponentCollection *estimatedScore = NULL) const;

  void EvaluateTranslationOptionListWithSourceContext(const InputType &input
      , const TranslationOptionList &translationOptionList) const;

  FFState* EvaluateWhenApplied(
    const Hypothesis& cur_hypo,
    const FFState* prev_state,
    ScoreComponentCollection* accumulator) const;
  FFState* EvaluateWhenApplied(
    const ChartHypothesis& /* cur_hypo */,
    int /* featureID - used to index the state in the previous hypotheses */,
    ScoreComponentCollection* accumulator) const;

  void SetParameter(const std::string& key, const std::string& value);

};


}

