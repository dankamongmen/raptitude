// test_matching.cc
//
//   Copyright (C) 2008 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.

#include <cppunit/extensions/HelperMacros.h>

#include <generic/apt/matching/compare_patterns.h>
#include <generic/apt/matching/parse.h>
#include <generic/apt/matching/pattern.h>
#include <generic/apt/matching/serialize.h>

#include <cwidget/generic/util/ssprintf.h>

using namespace aptitude::matching;
using cwidget::util::ref_ptr;
using cwidget::util::ssprintf;

namespace
{
  // We test this several ways.
  //
  // We test that the expected pattern equals itself.
  //
  // We test that patterns which don't have the same expected
  // serialization don't equal each other.
  //
  // We test that the comparison function is a total order on the test
  // set.
  //
  // We test that the input pattern parses to the expected pattern.
  //
  // We test that the expected pattern and the input pattern's parse
  // serialize to the expected serialization.
  //
  // We test that the expected serialization parses to the expected
  // pattern.
  struct pattern_test
  {
    std::string input_pattern;
    std::string expected_serialization;
    ref_ptr<pattern> expected_pattern;
  };

  pattern_test test_patterns[] = {
    { "?archive(^asdf.*asdf$)", "?archive(\"^asdf.*asdf$\")",
      pattern::make_archive("^asdf.*asdf$") },

    { "?action(install)", "?action(install)",
      pattern::make_action(pattern::action_install) },

    { "?action(upgrade)", "?action(upgrade)",
      pattern::make_action(pattern::action_upgrade) },

    { "?action(downgrade)", "?action(downgrade)",
      pattern::make_action(pattern::action_downgrade) },

    { "?action(remove)", "?action(remove)",
      pattern::make_action(pattern::action_remove) },

    { "?action(purge)", "?action(purge)",
      pattern::make_action(pattern::action_purge) },

    { "?action(reinstall)", "?action(reinstall)",
      pattern::make_action(pattern::action_reinstall) },

    { "?action(hold)", "?action(hold)",
      pattern::make_action(pattern::action_hold) },

    { "?action(keep)", "?action(keep)",
      pattern::make_action(pattern::action_keep) },

    { "?all-versions(~nelba~|a~\"ble)", "?all-versions(?name(\"elba|a\\\"ble\"))",
      pattern::make_all_versions(pattern::make_name("elba|a\"ble")) }
  };

  const int num_test_patterns = sizeof(test_patterns) / sizeof(test_patterns[0]);
}

class MatchingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MatchingTest);

  CPPUNIT_TEST(testTotalOrder);
  CPPUNIT_TEST(testParse);
  CPPUNIT_TEST(testParseThenSerialize);
  CPPUNIT_TEST(testSerialize);
  CPPUNIT_TEST(testSerializationParse);

  CPPUNIT_TEST_SUITE_END();

  class intArray2d
  {
    int *entries;
    int num_rows;
    int num_cols;

    intArray2d(const intArray2d &);

  public:
    intArray2d(int _num_rows, int _num_cols)
      : entries(new int[_num_rows * _num_cols]),
	num_rows(_num_rows), num_cols(_num_cols)
    {
    }

    ~intArray2d()
    {
      delete[] entries;
    }

    int &operator()(int row, int col)
    {
      return entries[row * num_cols + col];
    }

    const int &operator()(int row, int col) const
    {
      return entries[row * num_cols + col];
    }
  };

public:
  void testTotalOrder()
  {
    intArray2d comparisons(num_test_patterns, num_test_patterns);
    for(int i = 0; i < num_test_patterns; ++i)
      for(int j = 0; j < num_test_patterns; ++j)
	comparisons(i, j) = compare_patterns(test_patterns[i].expected_pattern,
					     test_patterns[j].expected_pattern);

    // We need to verify three properties:
    //
    // (1) Reflexivity: identical patterns (ones
    //     that have identical serialization) should
    //     be equal.
    // (2) Antisymmetry: if a < b then b > a.  If
    //     a = b then b = a and the patterns are
    //     identical.
    // (3) Transitivity: if a <= b and b <= c,
    //     then a <= c.  if a >= b and b >= c,
    //     then a >= c.

    // Reflexivity.
    for(int i = 0; i < num_test_patterns; ++i)
      for(int j = 0; j < num_test_patterns; ++j)
	{
	  if(test_patterns[i].expected_serialization ==
	     test_patterns[j].expected_serialization)
	    CPPUNIT_ASSERT_EQUAL(0, comparisons(i, j));
	  else
	    CPPUNIT_ASSERT_MESSAGE(ssprintf("Comparing %s to %s",
					    serialize_pattern(test_patterns[i].expected_pattern).c_str(),
					    serialize_pattern(test_patterns[j].expected_pattern).c_str()),
			   0 != comparisons(i, j));
	}

    // Antisymmetry.
    for(int i = 0; i < num_test_patterns; ++i)
      for(int j = 0; j < num_test_patterns; ++j)
	{
	  const int ij = comparisons(i, j);
	  const int ji = comparisons(j, i);

	  if(ij < 0)
	    CPPUNIT_ASSERT_MESSAGE(ssprintf("Comparing %s to %s",
					    serialize_pattern(test_patterns[j].expected_pattern).c_str(),
					    serialize_pattern(test_patterns[i].expected_pattern).c_str()),
				   ji > 0);
	  else if(ij > 0)
	    CPPUNIT_ASSERT_MESSAGE(ssprintf("Comparing %s to %s",
					    serialize_pattern(test_patterns[i].expected_pattern).c_str(),
					    serialize_pattern(test_patterns[j].expected_pattern).c_str()),
				   ji < 0);
	  else
	    CPPUNIT_ASSERT_EQUAL(test_patterns[i].expected_serialization,
				 test_patterns[j].expected_serialization);
	}

    for(int i = 0; i < num_test_patterns; ++i)
      for(int j = 0; j < num_test_patterns; ++j)
	for(int k = 0; k < num_test_patterns; ++k)
	  {
	    int ij = comparisons(i, j);
	    int jk = comparisons(j, k);
	    int ik = comparisons(i, k);

	    if(ij <= 0 && jk <= 0)
	      CPPUNIT_ASSERT_MESSAGE(ssprintf("Comparing %s to %s",
					      serialize_pattern(test_patterns[i].expected_pattern).c_str(),
					      serialize_pattern(test_patterns[k].expected_pattern).c_str()),
			     ik <= 0);

	    if(ij >= 0 && jk >= 0)
	      CPPUNIT_ASSERT_MESSAGE(ssprintf("Comparing %s to %s",
					      serialize_pattern(test_patterns[i].expected_pattern).c_str(),
					      serialize_pattern(test_patterns[k].expected_pattern).c_str()),
			     ik >= 0);
	  }
  }

  void testParse()
  {
    for(int i = 0; i < num_test_patterns; ++i)
      {
	const pattern_test &test(test_patterns[i]);

	ref_ptr<pattern> parsed(parse(test.input_pattern));
	CPPUNIT_ASSERT(parsed.valid());

	CPPUNIT_ASSERT_EQUAL_MESSAGE(ssprintf("Comparing %s to %s",
					      serialize_pattern(parsed).c_str(),
					      serialize_pattern(test.expected_pattern).c_str()),
				     0, compare_patterns(parsed,
							 test.expected_pattern));
      }
  }

  void testParseThenSerialize()
  {
    for(int i = 0; i < num_test_patterns; ++i)
      {
	const pattern_test &test(test_patterns[i]);

	ref_ptr<pattern> parsed(parse(test.input_pattern));
	CPPUNIT_ASSERT(parsed.valid());

	std::string serialized(serialize_pattern(parsed));

	CPPUNIT_ASSERT_EQUAL(test.expected_serialization,
			     serialized);
      }
  }

  void testSerialize()
  {
    for(int i = 0; i < num_test_patterns; ++i)
      {
	const pattern_test &test(test_patterns[i]);

	std::string serialized(serialize_pattern(test.expected_pattern));

	CPPUNIT_ASSERT_EQUAL(test.expected_serialization,
			     serialized);
      }
  }

  void testSerializationParse()
  {
    for(int i = 0; i < num_test_patterns; ++i)
      {
	const pattern_test &test(test_patterns[i]);

	ref_ptr<pattern> parsed(parse(test.expected_serialization));
	CPPUNIT_ASSERT(parsed.valid());

	CPPUNIT_ASSERT_EQUAL_MESSAGE(ssprintf("Comparing %s and %s",
					      serialize_pattern(parsed).c_str(),
					      serialize_pattern(test.expected_pattern).c_str()),
				     0,
				     compare_patterns(parsed,
						      test.expected_pattern));
      }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MatchingTest);
