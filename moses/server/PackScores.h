// -*- mode: c++; indent-tabs-mode: nil; tab-width:2  -*-
#ifdef HAVE_XMLRPC_C
#pragma once
#include <xmlrpc-c/base.hpp>
#include "moses/FF/FeatureFunction.h"
#include "moses/ScoreComponentCollection.h"

namespace Moses {

xmlrpc_c::value 
PackScores(ScoreComponentCollection const& S);

}


#endif
