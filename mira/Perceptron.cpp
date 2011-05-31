/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2010 University of Edinburgh

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

#include "Optimiser.h"

using namespace Moses;
using namespace std;

namespace Mira {

vector<int> Perceptron::updateWeightsHopeFear(ScoreComponentCollection& currWeights,
		const vector< vector<ScoreComponentCollection> >& featureValuesHope,
		const vector< vector<ScoreComponentCollection> >& featureValuesFear,
		const vector< vector<float> >& dummy1,
		const vector< vector<float> >& dummy2,
		const vector< size_t> dummy3,
		float perceptron_learning_rate,
		float dummy4,
		size_t rank,
		size_t epoch,
		bool dummy5)
{
	cerr << "hope: " << featureValuesHope[0][0] << endl;
	cerr << "fear: " << featureValuesFear[0][0] << endl;
	ScoreComponentCollection featureValueDiff = featureValuesHope[0][0];
	featureValueDiff.MinusEquals(featureValuesFear[0][0]);
	cerr << "hope - fear: " << featureValueDiff << endl;
	featureValueDiff.MultiplyEquals(perceptron_learning_rate);
	currWeights.PlusEquals(featureValueDiff);

	vector<int> update_status;
	update_status.push_back(0);
	update_status.push_back(0);
	update_status.push_back(0);
	return update_status;
}

}

