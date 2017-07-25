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

/********************************************************************/
/*                                                                  */
/* Module: LevelDbNgramTableTest                                   */
/*                                                                  */
/* Definitions file: LevelDbNgramTableTest.cc                      */
/*                                                                  */
/********************************************************************/


//--------------- Include files --------------------------------------

#include "LevelDbNgramTableTest.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( LevelDbNgramTableTest );

//--------------- LevelDbNgramTableTest class functions
//

//---------------------------------------
void LevelDbNgramTableTest::setUp()
{
  tab = new LevelDbNgramTable();
  tab->init(dbName);
}

//---------------------------------------
void LevelDbNgramTableTest::tearDown()
{
  tab->drop();
  delete tab;
}

//---------------------------------------
Vector<WordIndex> LevelDbNgramTableTest::getVector(string phrase) {
  Vector<WordIndex> v;

  for(unsigned int i = 0; i < phrase.size(); i++) {
    v.push_back(phrase[i]);
  }

  return(v);
}

//---------------------------------------
void LevelDbNgramTableTest::testStoreAndRestoreSrcInfo()
{
  Vector<WordIndex> s1 = getVector("Ulica Krancowa");
  Vector<WordIndex> s2 = getVector("Ulica Sienkiewicza");
  Count cs1 = Count(1);
  Count cs2 = Count(23);
  tab->clear();
  tab->addSrcInfo(s1, cs1);
  tab->addSrcInfo(s2, cs2);

  bool found;
  Count s1_count = tab->getSrcInfo(s1, found);
  Count s2_count = tab->getSrcInfo(s2, found);

  CPPUNIT_ASSERT( (int) s1_count.get_c_s() == 1 );
  CPPUNIT_ASSERT( (int) s2_count.get_c_s() == 23 );
}

//---------------------------------------
void LevelDbNgramTableTest::testKeyVectorConversion()
{
  Vector<WordIndex> s;
  s.push_back(3);
  s.push_back(4);
  s.push_back(112175);
  s.push_back(90664);
  s.push_back(143);
  s.push_back(749);
  s.push_back(748);

  CPPUNIT_ASSERT( tab->keyToVector(tab->vectorToKey(s)) == s );
}

//---------------------------------------
void LevelDbNgramTableTest::testAddTableEntry()
{
  Vector<WordIndex> s = getVector("race around Narie");
  WordIndex t = 10688;
  Count s_count = Count(7);
  Count st_count = Count(5);
  im_pair<Count, Count> ppi;
  ppi.first = s_count;
  ppi.second = st_count;

  tab->clear();
  tab->addTableEntry(s, t, ppi);

  CPPUNIT_ASSERT( (int) tab->cSrc(s).get_c_s() == 7 );
  CPPUNIT_ASSERT( (int) tab->cSrcTrg(s, t).get_c_st() == 5 );
}

//---------------------------------------
void LevelDbNgramTableTest::testIncrCountsOfEntryLog()
{
  Vector<WordIndex> s = getVector("Narie lake");
  WordIndex t1 = 140991;
  WordIndex t2 = 230689;
  LogCount c_init = LogCount(log(3));
  LogCount c = LogCount(log(17));

  tab->clear();
  tab->incrCountsOfEntryLog(s, t1, c_init);
  tab->incrCountsOfEntryLog(s, t2, c);

  CPPUNIT_ASSERT( (int) tab->cSrcTrg(s, t1).get_c_st() == 3 );
  CPPUNIT_ASSERT( (int) tab->cSrcTrg(s, t2).get_c_st() == 17 );
}

//---------------------------------------
void LevelDbNgramTableTest::testGetEntriesForTarget()
{
  LevelDbNgramTable::SrcTableNode node;
  Vector<WordIndex> s1_1;
  s1_1.push_back(1);
  Vector<WordIndex> s1_2;
  s1_2.push_back(1);
  s1_2.push_back(2);
  WordIndex t1_1 = 1;
  WordIndex t1_2 = 5;
  Vector<WordIndex> s2;
  s2.push_back(3);
  WordIndex t2 = 2;
  LogCount c = LogCount(log(1));

  tab->clear();
  tab->incrCountsOfEntryLog(s1_1, t1_1, c);
  tab->incrCountsOfEntryLog(s1_2, t1_1, c);
  tab->incrCountsOfEntryLog(s1_1, t1_2, c);
  tab->incrCountsOfEntryLog(s2, t2, c);

  bool result;
  // Looking for phrases with 1 as a target
  result = tab->getEntriesForTarget(t1_1, node);
  CPPUNIT_ASSERT( result );
  CPPUNIT_ASSERT( node.size() == 2 );

  // Looking for phrases with 5 as a target
  result = tab->getEntriesForTarget(t1_2, node);
  CPPUNIT_ASSERT( result );
  CPPUNIT_ASSERT( node.size() == 1 );

  // Looking for phrases with 2 as a target
  result = tab->getEntriesForTarget(t2, node);
  CPPUNIT_ASSERT( result );
  CPPUNIT_ASSERT( node.size() == 1 );

  // '9' key shoud not be found
  result = tab->getEntriesForTarget(9, node);
  CPPUNIT_ASSERT( !result );
}

//---------------------------------------
void LevelDbNgramTableTest::testRetrievingSubphrase()
{
  //  TEST:
  //    Accessing element with the subphrase should return count 0
  //
  bool found;
  Vector<WordIndex> s1;
  s1.push_back(1);
  s1.push_back(1);
  Vector<WordIndex> s2 = s1;
  s2.push_back(1);
  WordIndex t1 = 2;
  WordIndex t2 = 3;


  tab->clear();
  tab->incrCountsOfEntryLog(s2, t1, LogCount(log(2)));
  Count c = tab->getSrcTrgInfo(s2, t2, found);

  CPPUNIT_ASSERT( !found );
  CPPUNIT_ASSERT( (int) c.get_c_s() == 0);

  tab->incrCountsOfEntryLog(s2, t2, LogCount(log(3)));
  c = tab->getSrcTrgInfo(s2, t2, found);

  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( (int) c.get_c_s() == 3);

  c = tab->getSrcInfo(s1, found);

  CPPUNIT_ASSERT( !found );
  CPPUNIT_ASSERT( (int) c.get_c_s() == 0);


}

//---------------------------------------
/*void LevelDbNgramTableTest::testRetrieveNonLeafPhrase()
{
  //  TEST:
  //    Phrases with count > 0 and not stored in the leaves
  //    should be also retrieved
  //
  bool found;
  LevelDbNgramTable::SrcTableNode node;
  Vector<WordIndex> s = getVector("Hello");
  Vector<WordIndex> t1 = getVector("Buenos Dias");
  Vector<WordIndex> t2 = getVector("Buenos");

  Count c = Count(1);
  
  tab->clear();
  tab->incrCountsOfEntry(s, t1, c);
  tab->incrCountsOfEntry(s, t2, c);

  // Check phrases and their counts
  // Phrase pair 1
  c = tab->getSrcTrgInfo(s, t1, found);

  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( (int) c.get_c_s() == 1);
  // Phrase pair 2
  c = tab->getSrcTrgInfo(s, t2, found);

  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( (int) c.get_c_s() == 1);

  // Looking for phrases for which 'Buenos' is translation
  found = tab->getEntriesForTarget(t2, node);
  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 1 );
}*/

//---------------------------------------
void LevelDbNgramTableTest::testGetEntriesForSource()
{
  //  TEST:
  //    Find translations for the source phrase
  //    WARNING: Src phrases has to be present to get results
  //
  bool found;
  LevelDbNgramTable::TrgTableNode node;
  Vector<WordIndex> s1 = getVector("jezioro Narie");
  WordIndex t1_1 = 230689;
  WordIndex t1_2 = 140991;
  Vector<WordIndex> s2 = getVector("jezioro Krzywe");
  WordIndex t2_1 = 110735;
  Vector<WordIndex> s3 = getVector("jezioro Jeziorak");
  WordIndex t3_1 = 5;
  WordIndex t3_2 = 10;

  Count c = Count(1);
  
  // Prepare data struture
  tab->clear();
  // Add Narie phrases
  tab->addSrcInfo(s1, c + c);
  tab->incrCountsOfEntry(s1, t1_1, c);
  tab->incrCountsOfEntry(s1, t1_2, c);
  // Add Skiertag phrases
  tab->addSrcInfo(s2, c);
  tab->incrCountsOfEntry(s2, t2_1, c);
  // Add Jeziorak phrases
  tab->addSrcInfo(s3, c + c);
  tab->incrCountsOfEntry(s3, t3_1, c);
  tab->incrCountsOfEntry(s3, t3_2, c);

  // Looking for translations
  // Narie phrases
  found = tab->getEntriesForSource(s1, node);
  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 2 );
  // Skiertag phrases
  found = tab->getEntriesForSource(s2, node);
  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 1 );
  // Jeziorak phrases
  found = tab->getEntriesForSource(s3, node);
  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 2 );
}

//---------------------------------------
/*void LevelDbNgramTableTest::testRetrievingEntriesWithCountEqualZero()
{
  // TEST:
  //   Function getEntriesForTarget for retrieving entries should skip
  //   elements with count equals 0
  //
  bool found;
  LevelDbNgramTable::SrcTableNode node;
  Vector<WordIndex> s1 = getVector("Palac Dohnow");
  Vector<WordIndex> s2 = getVector("Palac Dohnow w Moragu");
  Vector<WordIndex> t = getVector("Dohn's Palace");
  
  tab->clear();
  tab->incrCountsOfEntry(s1, t, Count(1));
  tab->incrCountsOfEntry(s2, t, Count(0));

  found = tab->getEntriesForTarget(t, node);

  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 1 );
}*/

//---------------------------------------
void LevelDbNgramTableTest::testGetNbestForSrc()
{
  //  TEST:
  //    Check if method getNbestForSrc returns correct elements
  //    WARNING: Both - (s) and (s,t) phrases have to present to
  //    retrieve results
  //
  bool found;
  NbestTableNode<WordIndex> node;
  NbestTableNode<WordIndex>::iterator iter;

  // Fill leveldb with data
  Vector<WordIndex> s;
  s.push_back(1);

  WordIndex t1 = 1;
  WordIndex t2 = 2;
  WordIndex t3 = 3;
  WordIndex t4 = 4;
  
  tab->clear();
  tab->addSrcInfo(s, Count(10));
  tab->incrCountsOfEntryLog(s, t1, LogCount(log(4)));
  tab->incrCountsOfEntryLog(s, t2, LogCount(log(2)));
  tab->incrCountsOfEntryLog(s, t3, LogCount(log(3)));
  tab->incrCountsOfEntryLog(s, t4, LogCount(log(1)));

  // Returned elements should not exceed number of elements
  // in the structure
  found = tab->getNbestForSrc(s, node);
  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( node.size() == 4 );

  // If there are more available elements, at the beginning
  // the most frequent targets should be returned
  iter = node.begin();
  CPPUNIT_ASSERT( iter->second == t1 );
  iter++;
  CPPUNIT_ASSERT( iter->second == t3 );
  iter++;
  CPPUNIT_ASSERT( iter->second == t2 );
  iter++;
  CPPUNIT_ASSERT( iter->second == t4 );
}

//---------------------------------------
/*void LevelDbNgramTableTest::testAddSrcTrgInfo()
{
  //  TEST:
  //    Check if two keys were added (for (s, t) and (t, s) vectors)
  //    and if their values are the same
  //
  bool found;

  Vector<WordIndex> s = getVector("jezioro Skiertag");
  Vector<WordIndex> t = getVector("Skiertag lake");
  
  tab->clear();
  tab->addSrcTrgInfo(s, t, Count(1));

  Count src_trg_count = tab->cSrcTrg(s, t);
  Count trg_src_count = tab->getInfo(tab->getTrgSrc(s, t), found);

  CPPUNIT_ASSERT( found );
  CPPUNIT_ASSERT( (int) src_trg_count.get_c_s() == 1 );
  CPPUNIT_ASSERT( (int) src_trg_count.get_c_s() == (int) trg_src_count.get_c_s() );
}*/

//---------------------------------------
void LevelDbNgramTableTest::testIteratorsLoop()
{
  //  TEST:
  //    Check basic implementation of iterators - functions
  //    begin(), end() and operators (++ postfix, *).
  //

  Vector<WordIndex> s1;
  s1.push_back(1);
  WordIndex t1 = 2;

  Vector<WordIndex> s2;
  s2.push_back(1);
  s2.push_back(2);
  WordIndex t2 = 3;
  
  tab->clear();
  tab->addSrcInfo(s1, Count(2));
  tab->incrCountsOfEntryLog(s1, t1, LogCount(log(2)));
  tab->incrCountsOfEntryLog(s2, t2, LogCount(log(1)));

  CPPUNIT_ASSERT(tab->begin() != tab->end());
  CPPUNIT_ASSERT(tab->begin() != tab->begin());

  int i = 0;
  const int MAX_ITER = 10;

  // Construct dictionary to record results returned by iterator
  // Dictionary structure: (key, (total count value, number of occurences))
  map<Vector<WordIndex>, pair<Count, Count> > d;
  d[s1] = make_pair(Count(0), Count(0));
  d[tab->getSrcTrg(s1, t1)] = make_pair(Count(0), Count(0));
  d[tab->getSrcTrg(s2, t2)] = make_pair(Count(0), Count(0));

  for(LevelDbNgramTable::const_iterator iter = tab->begin(); iter != tab->end() && i < MAX_ITER; iter++, i++)
  {
    pair<Vector<WordIndex>, Count> x = *iter;
    d[x.first].first += x.second;
    d[x.first].second += 1;
  }

  // Check if element returned by iterator is correct
  CPPUNIT_ASSERT(d.size() == 3);
  CPPUNIT_ASSERT(d[s1].first.get_c_s() == 2);
  CPPUNIT_ASSERT(d[s1].second.get_c_s() == 1);
  CPPUNIT_ASSERT(d[tab->getSrcTrg(s1, t1)].first.get_c_st() == 2);
  CPPUNIT_ASSERT(d[tab->getSrcTrg(s1, t1)].second.get_c_s() == 1);
  CPPUNIT_ASSERT(d[tab->getSrcTrg(s2, t2)].first.get_c_st() == 1);
  CPPUNIT_ASSERT(d[tab->getSrcTrg(s2, t2)].second.get_c_s() == 1);

  CPPUNIT_ASSERT( i == 3 );
}

//---------------------------------------
/*void LevelDbNgramTableTest::testIteratorsOperatorsPlusPlusStar()
{
  //  TEST:
  //    Check basic implementation of iterators - function
  //    begin() and operators (++ prefix, ++ postfix, *, ->).
  //
  bool found = true;

  Vector<WordIndex> s = getVector("zamek krzyzacki w Malborku");
  Vector<WordIndex> t = getVector("teutonic castle in Malbork");
  
  tab->clear();
  tab->incrCountsOfEntry(s, t, Count(2));

  // Construct dictionary to record results returned by iterator
  // Dictionary structure: (key, (total count value, number of occurences))
  map<Vector<WordIndex>, pair<int, int> > d;
  d[tab->getSrc(s)] = make_pair(0, 0);
  d[t] = make_pair(0, 0);
  d[tab->getTrgSrc(s, t)] = make_pair(0, 0);
 
  for(LevelDbNgramTable::const_iterator iter = tab->begin(); iter != tab->end(); found = (iter++))
  {
    CPPUNIT_ASSERT( found );
    pair<Vector<WordIndex>, int> x = *iter;
    d[x.first].first += x.second;
    d[x.first].second++;
  }

  // Iterating beyond the last element should return FALSE value
  CPPUNIT_ASSERT( !found );

  // Check if element returned by iterator is correct
  CPPUNIT_ASSERT(d.size() == 3);
  CPPUNIT_ASSERT(d[tab->getSrc(s)].first == 2);
  CPPUNIT_ASSERT(d[tab->getSrc(s)].second == 1);
  CPPUNIT_ASSERT(d[t].first == 2);
  CPPUNIT_ASSERT(d[t].second == 1);
  CPPUNIT_ASSERT(d[tab->getTrgSrc(s, t)].first == 2);
  CPPUNIT_ASSERT(d[tab->getTrgSrc(s, t)].second == 1);
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testIteratorsOperatorsEqualNotEqual()
{
  //  TEST:
  //    Check basic implementation of iterators - operators == and !=
  //
  Vector<WordIndex> s = getVector("kemping w Kretowinach");
  Vector<WordIndex> t = getVector("camping Kretowiny");
  
  tab->clear();
  tab->incrCountsOfEntry(s, t, Count(1));

  LevelDbNgramTable::const_iterator iter1 = tab->begin();
  iter1++;
  LevelDbNgramTable::const_iterator iter2 = tab->begin();
  
  CPPUNIT_ASSERT( iter1 == iter1 );
  CPPUNIT_ASSERT( !(iter1 != iter1) );
  CPPUNIT_ASSERT( !(iter1 == iter2) );
  CPPUNIT_ASSERT( iter1 != iter2 );
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testSize()
{
  //  TEST:
  //    Check if number of elements in the levelDB is returned correctly
  //
  tab->clear();
  CPPUNIT_ASSERT( tab->size() == 0 );  // Collection after cleaning should be empty
  
  // Fill leveldb with data
  tab->incrCountsOfEntry(getVector("kemping w Kretowinach"), getVector("camping Kretowiny"), Count(1));
  tab->incrCountsOfEntry(getVector("kemping w Kretowinach"), getVector("camping in Kretowiny"), Count(2));

  CPPUNIT_ASSERT( tab->size() == 5 );

  tab->clear();
  CPPUNIT_ASSERT( tab->size() == 0 );  // Collection after cleaning should be empty

  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mr Car"), Count(1));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Pan Samochodzik"), Count(4));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mister Automobile"), Count(20));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mr Automobile"), Count(24));

  CPPUNIT_ASSERT( tab->size() == 9 );

  tab->incrCountsOfEntry(getVector("Pierwsza przygoda Pana Samochodzika"),
                         getVector("First Adventure of Mister Automobile"), Count(5));
  tab->incrCountsOfEntry(getVector("Pierwsza przygoda Pana Samochodzika"),
                         getVector("First Adventure of Pan Samochodzik"), Count(7));


  CPPUNIT_ASSERT( tab->size() == 9 + 5 );

}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testLoadingLevelDb()
{
  //  TEST:
  //    Check restoring levelDB from disk
  //
  bool result;
  
  // Fill leveldb with data
  tab->clear();
  tab->incrCountsOfEntry(getVector("kemping w Kretowinach"), getVector("camping Kretowiny"), Count(1));
  tab->incrCountsOfEntry(getVector("kemping w Kretowinach"), getVector("camping in Kretowiny"), Count(2));

  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mr Car"), Count(1));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Pan Samochodzik"), Count(4));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mister Automobile"), Count(20));
  tab->incrCountsOfEntry(getVector("Pan Samochodzik"), getVector("Mr Automobile"), Count(24));

  tab->incrCountsOfEntry(getVector("Pierwsza przygoda Pana Samochodzika"),
                         getVector("First Adventure of Mister Automobile"), Count(5));
  tab->incrCountsOfEntry(getVector("Pierwsza przygoda Pana Samochodzika"),
                         getVector("First Adventure of Pan Samochodzik"), Count(7));

  unsigned int original_size = tab->size();
   
  // Load structure
  result = tab->load(dbName);
  CPPUNIT_ASSERT( result == THOT_OK);
  CPPUNIT_ASSERT( tab->size() == original_size );
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testLoadedDataCorrectness()
{
  //  TEST:
  //    Check if the data restored from disk
  //    contains all stored items and correct counts
  //
  bool result;
  
  // Fill levelDB with data
  tab->clear();

  // Define vectors
  Vector<WordIndex> s1 = getVector("Pan Samochodzik");
  Vector<WordIndex> t1_1 = getVector("Mr Car");
  Vector<WordIndex> t1_2 = getVector("Pan Samochodzik");
  Vector<WordIndex> t1_3 = getVector("Mister Automobile");
  Vector<WordIndex> t1_4 = getVector("Mr Automobile");

  Vector<WordIndex> s2 = getVector("Pan Samochodzik i templariusze");
  Vector<WordIndex> t2_1 = getVector("Mister Automobile and the Knights Templar");
  Vector<WordIndex> t2_2 = getVector("Pan Samochodzik and the Knights Templar");

  Vector<WordIndex> s3 = getVector("Pan Samochodzik i niesamowity dwor");
  Vector<WordIndex> t3_1 = getVector("Mister Automobile and the Unearthly Mansion");
  Vector<WordIndex> t3_2 = getVector("Pan Samochodzik and the Unearthly Mansion");

  // Insert data to levelDB
  tab->incrCountsOfEntry(s1, t1_1, Count(1));
  tab->incrCountsOfEntry(s1, t1_2, Count(2));
  tab->incrCountsOfEntry(s1, t1_3, Count(4));
  tab->incrCountsOfEntry(s1, t1_4, Count(8));

  tab->incrCountsOfEntry(s2, t2_1, Count(16));
  tab->incrCountsOfEntry(s2, t2_2, Count(32));

  tab->incrCountsOfEntry(s3, t3_1, Count(64));
  tab->incrCountsOfEntry(s3, t3_2, Count(128));

  unsigned int original_size = tab->size();
   
  // Load structure
  result = tab->load(dbName);
  CPPUNIT_ASSERT( result == THOT_OK );
  CPPUNIT_ASSERT( tab->size() == original_size );

  // Check count values
  CPPUNIT_ASSERT( tab->cSrc(s1).get_c_s() == 1 + 2 + 4 + 8 );
  CPPUNIT_ASSERT( tab->cTrg(t1_1).get_c_s() == 1 );
  CPPUNIT_ASSERT( tab->cTrg(t1_2).get_c_s() == 2 );
  CPPUNIT_ASSERT( tab->cTrg(t1_3).get_c_s() == 4 );
  CPPUNIT_ASSERT( tab->cTrg(t1_4).get_c_s() == 8 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_1).get_c_st() == 1 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_2).get_c_st() == 2 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_3).get_c_st() == 4 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_4).get_c_st() == 8 );

  CPPUNIT_ASSERT( tab->cSrc(s2).get_c_s() == 16 + 32 );
  CPPUNIT_ASSERT( tab->cTrg(t2_1).get_c_s() == 16 );
  CPPUNIT_ASSERT( tab->cTrg(t2_2).get_c_s() == 32 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s2, t2_1).get_c_st() == 16 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s2, t2_2).get_c_st() == 32 );

  CPPUNIT_ASSERT( tab->cSrc(s3).get_c_s() == 64 + 128 );
  CPPUNIT_ASSERT( tab->cTrg(t3_1).get_c_s() == 64 );
  CPPUNIT_ASSERT( tab->cTrg(t3_2).get_c_s() == 128 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s3, t3_1).get_c_st() == 64 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s3, t3_2).get_c_st() == 128 );

  tab->clear();  // Remove data
  CPPUNIT_ASSERT( tab->size() == 0 );
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testSubkeys()
{
  //  TEST:
  //    Check if subkeys are stored correctly
  //
 
  // Fill levelDB with data
  tab->clear();

  // Define vectors
  Vector<WordIndex> s1 = getVector("Pan Samochodzik");
  Vector<WordIndex> t1_1 = getVector("Mr Car");
  Vector<WordIndex> t1_2 = getVector("Pan");
  Vector<WordIndex> t1_3 = getVector("Mr");

  Vector<WordIndex> s2 = getVector("Pan");
  Vector<WordIndex> t2_1 = getVector("Mister");
  Vector<WordIndex> t2_2 = getVector("Mr");

  // Insert data to levelDB
  tab->incrCountsOfEntry(s1, t1_1, Count(1));
  tab->incrCountsOfEntry(s1, t1_2, Count(2));
  tab->incrCountsOfEntry(s1, t1_3, Count(4));

  tab->incrCountsOfEntry(s2, t2_1, Count(8));
  tab->incrCountsOfEntry(s2, t2_2, Count(16));
  

  CPPUNIT_ASSERT( tab->size() == 11 );

  // Check count values
  CPPUNIT_ASSERT( tab->cSrc(s1).get_c_s() == 1 + 2 + 4 );
  CPPUNIT_ASSERT( tab->cTrg(t1_1).get_c_s() == 1 );
  CPPUNIT_ASSERT( tab->cTrg(t1_2).get_c_s() == 2 );
  CPPUNIT_ASSERT( tab->cTrg(t1_3).get_c_s() == 4 + 16 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_1).get_c_st() == 1 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_2).get_c_st() == 2 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s1, t1_3).get_c_st() == 4 );

  CPPUNIT_ASSERT( tab->cSrc(s2).get_c_s() == 8 + 16 );
  CPPUNIT_ASSERT( tab->cTrg(t2_1).get_c_s() == 8 );
  CPPUNIT_ASSERT( tab->cTrg(t2_2).get_c_s() == 4 + 16 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s2, t2_1).get_c_st() == 8 );
  CPPUNIT_ASSERT( tab->cSrcTrg(s2, t2_2).get_c_st() == 16 );
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::test32bitRange()
{
  //  TEST:
  //    Check if levelDB supports codes from positive integer range
  //
  tab->clear();

  Vector<WordIndex> minVector, maxVector;

  minVector.push_back(0);
  maxVector.push_back(0x7FFFFFFE);

  // Insert data to levelDB and check their correctness
  tab->incrCountsOfEntry(minVector, maxVector, Count(20));
  CPPUNIT_ASSERT( tab->size() == 3 );
  CPPUNIT_ASSERT( (int) tab->cSrcTrg(minVector, maxVector).get_c_st() == 20 );
}*/

//---------------------------------------
/*void LevelDbNgramTableTest::testByteMax()
{
  //  TEST:
  //    Check if items with maximum byte value are added correctly
  //
  tab->clear();

  Vector<WordIndex> s, t;
  s.push_back(201);
  s.push_back(8);
  t.push_back(255);

  // Insert data and check their correctness
  tab->incrCountsOfEntry(s, t, Count(1));
  CPPUNIT_ASSERT( tab->size() == 3 );
  CPPUNIT_ASSERT( (int) tab->cSrcTrg(s, t).get_c_st() == 1 );
}*/