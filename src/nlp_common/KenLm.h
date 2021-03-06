/*
thot package for statistical machine translation
Copyright (C) 2013 Daniel Ortiz-Mart\'inez
 
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file KenLm.h
 * 
 * @brief Wrapper for kenlm.
 */

#ifndef _KenLm
#define _KenLm

//--------------- Include files --------------------------------------

#if HAVE_CONFIG_H
#  include <thot_config.h>
#endif /* HAVE_CONFIG_H */

#include "lm/model.hh"
#include "BaseNgramLM.h"
#include "ModelDescriptorUtils.h"
#include <algorithm>

//--------------- Constants ------------------------------------------


//--------------- typedefs -------------------------------------------


//--------------- function declarations ------------------------------


//--------------- Classes --------------------------------------------

//--------------- KenLm class

class KenLm: public BaseNgramLM<std::vector<WordIndex> >
{
 public:

  typedef BaseNgramLM<std::vector<WordIndex> >::LM_State LM_State;

      // Constructor
  KenLm();

        // Probability functions
  LgProb getNgramLgProb(WordIndex w,const std::vector<WordIndex>& vu);
      // returns the probability of an n-gram, uv[0] stores the n-1'th
      // word of the n-gram, uv[1] the n-2'th one and so on
  LgProb getNgramLgProbStr(std::string s,const std::vector<std::string>& rq);
      // returns the probability of an n-gram. Each string represents a
      // single word
  LgProb getLgProbEnd(const std::vector<WordIndex>& vu);
  LgProb getLgProbEndStr(const std::vector<std::string>& rq);

        // Probability functions using states
  bool getStateForWordSeq(const std::vector<WordIndex>& wordSeq,
                          std::vector<WordIndex>& state);
  void getStateForBeginOfSentence(std::vector<WordIndex> &state);
  LgProb getNgramLgProbGivenState(WordIndex w,
                                  std::vector<WordIndex> &state);
  LgProb getNgramLgProbGivenStateStr(std::string s,
                                     std::vector<WordIndex> &state);
  LgProb getLgProbEndGivenState(std::vector<WordIndex> &state);
      // In these functions, the state is updated once the
      // function is executed
   
      // Encoding-related functions
  bool existSymbol(std::string s)const;
  WordIndex addSymbol(std::string s);
  unsigned int getVocabSize(void);
  WordIndex stringToWordIndex(std::string s)const;
  std::string wordIndexToString(WordIndex w)const;
  WordIndex getBosId(bool &found)const;
  WordIndex getEosId(bool &found)const;
  bool loadVocab(const char *fileName);
      // Load encoding information given a prefix file name
  bool printVocab(const char *fileName);
      // Prints encoding information
  void clearVocab(void);
      // Clears encoding information
  
      // Functions to load and print the model
  bool load(const char *fileName);
  bool print(const char *fileName);
  unsigned int getNgramOrder(void);
  void setNgramOrder(int _ngramOrder);
  
      // size and clear functions
  size_t size(void);
  void clear(void);

      // Destructor
  virtual ~KenLm();
   
 protected:

  typedef lm::ngram::SortedVocabulary KlmVocabulary;
  typedef lm::ngram::TrieModel KenLangModel;

  KenLangModel* modelPtr;

      // Auxiliary functions
  bool load_kenlm_file(const char *fileName);
};

#endif
