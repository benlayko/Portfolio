#include "gtest/gtest.h"
#include "API.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

namespace{

class m_tree_test : public ::testing::Test
{
public:
  m_tree_t *tree_;
  virtual void SetUp()
  {
    tree_ = create_m_tree();
  }

  virtual void TearDown()
  {
    destroy_m_tree(tree_);
  }
};

TEST_F(m_tree_test, test_query_length)
{
  ASSERT_EQ(query_length(tree_), 0);
}


TEST_F(m_tree_test, test_insert_interval)
{
  insert_interval(tree_, 1, 0);
  ASSERT_EQ(query_length(tree_), 0);

  insert_interval(tree_, 1, 2);
  ASSERT_EQ(query_length(tree_), 1);

  insert_interval(tree_, 2, 4);
  ASSERT_EQ(query_length(tree_), 3);

  insert_interval(tree_, 6, 10);
  ASSERT_EQ(query_length(tree_), 7);

  insert_interval(tree_, 7, 8);
  ASSERT_EQ(query_length(tree_), 7);

  insert_interval(tree_, 7, 11);
  ASSERT_EQ(query_length(tree_), 8);

  insert_interval(tree_, -1, 1);
  ASSERT_EQ(query_length(tree_), 10);

  insert_interval(tree_, -5, -3);
  ASSERT_EQ(query_length(tree_), 12);

  insert_interval(tree_, -6, -4);
  ASSERT_EQ(query_length(tree_), 13);

  insert_interval(tree_, -7, 11);
  ASSERT_EQ(query_length(tree_), 18);
}

TEST_F(m_tree_test, test_delete_interval)
{
  delete_interval(tree_, 1, 2);
  ASSERT_EQ(query_length(tree_), 0);

  insert_interval(tree_, 1, 2);
  delete_interval(tree_, 1, 3);
  ASSERT_EQ(query_length(tree_), 1);
  delete_interval(tree_, 1, 2);
  ASSERT_EQ(query_length(tree_), 0);

  insert_interval(tree_, 1, 2);
  insert_interval(tree_, 2, 4);
  delete_interval(tree_, 1, 2);
  ASSERT_EQ(query_length(tree_), 2);

  insert_interval(tree_, 5, 9);
  delete_interval(tree_, 2, 4);
  ASSERT_EQ(query_length(tree_), 4);

  insert_interval(tree_, 6, 8);
  delete_interval(tree_, 5, 9);
  ASSERT_EQ(query_length(tree_), 2);

  insert_interval(tree_, 7, 10);
  delete_interval(tree_, 6, 8);
  ASSERT_EQ(query_length(tree_), 3);

  insert_interval(tree_, 6, 8);
  delete_interval(tree_, 7, 10);
  ASSERT_EQ(query_length(tree_), 2);

  insert_interval(tree_, 2, 10);
  delete_interval(tree_, 6, 8);
  ASSERT_EQ(query_length(tree_), 8);

  insert_interval(tree_, -1, 2);
  delete_interval(tree_, 2, 10);
  ASSERT_EQ(query_length(tree_), 3);

  delete_interval(tree_, -1, 2);
  ASSERT_EQ(query_length(tree_), 0);
}

TEST_F(m_tree_test, integration_test){
  int i;

  for(i=0; i< 50; i++ ){
    insert_interval( tree_, 2*i, 2*i +1 );
  }
  EXPECT_EQ(50, query_length(tree_)); // inserted first 50 intervals

  insert_interval( tree_, 0, 100 );
  EXPECT_EQ(100, query_length(tree_)); //inserted another interval

  for(i=1; i< 50; i++ ){
    insert_interval( tree_, 199 - (3*i), 200 ); /*[52,200] is longest*/
  }
  EXPECT_EQ(200, query_length(tree_)); // inserted further 49 intervals

  for(i=2; i< 50; i++ ){
    delete_interval(tree_, 2*i, 2*i +1 );
  }
  delete_interval(tree_,0,100);
  EXPECT_EQ(150, query_length(tree_)); //deleted some intervals

  insert_interval( tree_, 1,2 );
  for(i=49; i>0; i-- ){
    delete_interval( tree_, 199 - (3*i), 200 );
  }

  insert_interval( tree_, 0,2 );
  insert_interval( tree_, 1,5 );
  EXPECT_EQ(5, query_length(tree_)); //deleted some intervals,

  insert_interval( tree_, 0, 100 );
  EXPECT_EQ(100, query_length(tree_)); //inserted another interval

  for(i=0; i<=3000; i++ ){
      insert_interval( tree_, 2000+i, 3000+i );
  }
  EXPECT_EQ(4100, query_length(tree_)); //inserted 3000 intervals

  for(i=0; i<=3000; i++ ){
    delete_interval( tree_, 2000+i, 3000+i );
  }
  EXPECT_EQ(100, query_length(tree_)); //deleted 3000 intervals

  for(i=0; i<=100; i++ ){
    insert_interval( tree_, 10*i, 10*i+100 );
  }
  EXPECT_EQ(1100, query_length(tree_)); //inserted another 100 intervals

  delete_interval( tree_, 1,2 );
  delete_interval( tree_, 0,2 );
  delete_interval( tree_, 2,3 );
  delete_interval( tree_, 0,1 );
  delete_interval( tree_, 1,5 );
  EXPECT_EQ(1100, query_length(tree_)); //deleted some intervals

  for(i=0; i<= 100; i++ ){
    delete_interval(tree_, 10*i, 10*i+100);
  }
  delete_interval(tree_,0,100);
  EXPECT_EQ(0, query_length(tree_)); //deleted last interval

  for( i=0; i<100000; i++){
    insert_interval(tree_, i, 1000000);
  }
  ASSERT_EQ(1000000, query_length(tree_)); //inserted again 100000 intervals
}

TEST_F(m_tree_test, test_insert_complexity){
  int i=0;
  int sign=1;
  for (i=0; i<3000000; i++)
  {
    if (sign > 0 )
    {
      insert_interval(tree_, -1*i, 2*i);
    }
    else
    {
      insert_interval(tree_, -2*i, i);
    }

    sign *= -1;
  }
}

TEST_F(m_tree_test, test_delete_complexity){
  int i=0;
  for (i=0; i<3000000; i++)
  {
    insert_interval(tree_, 2*i, 2*i+1);
  }
  for (i=0; i<3000000; i++)
  {
    delete_interval(tree_, 2*i, 2*i+1);
  }
}

TEST_F(m_tree_test, test_query_complexity)
{
  int i=0;
  for (i=0; i<3000000; i++)
  {
    insert_interval(tree_, 3*i, 3*i+1);
  }
  for (i=0; i<1000000000; i++)
  {
    query_length(tree_);
  }
}

}

TEST_F(m_tree_test, test_print_submeasure)
{
  //TA will check the result manually.
  insert_interval(tree_, 1, 0);
  insert_interval(tree_, 1, 2);
  print_submeasures(tree_);
  insert_interval(tree_, 2, 4);
  insert_interval(tree_, 6, 10);
  print_submeasures(tree_);
  insert_interval(tree_, 7, 8);
  insert_interval(tree_, 7, 11);
  insert_interval(tree_, -1, 1);
  print_submeasures(tree_);
  insert_interval(tree_, -5, -3);
  print_submeasures(tree_);
  insert_interval(tree_, -6, -4);
  print_submeasures(tree_);
  insert_interval(tree_, -7, 11);
  print_submeasures(tree_);
}

TEST_F(m_tree_test, test_max_submeasure)
{
  insert_interval(tree_, 1, 0);
  insert_interval(tree_, 1, 2);

  int key_of_max_1 = max_submeasure(tree_);
  EXPECT_TRUE (key_of_max_1 == 1);

  insert_interval(tree_, 2, 4);
  insert_interval(tree_, 6, 10);

  int key_of_max_2 = max_submeasure(tree_);
  EXPECT_TRUE (key_of_max_2 == 6);

  insert_interval(tree_, 7, 8);
  insert_interval(tree_, 7, 11);

  int key_of_max_3 = max_submeasure(tree_);
  EXPECT_TRUE (key_of_max_3 == 2 || key_of_max_3 == 8);

  insert_interval(tree_, -1, 1);
  insert_interval(tree_, -5, -3);

  int key_of_max_4 = max_submeasure(tree_);
  EXPECT_TRUE (key_of_max_4 == -5 || key_of_max_4 == -1 || key_of_max_4 == 2 || key_of_max_4 == 8);

  insert_interval(tree_, -6, -4);
  insert_interval(tree_, -7, 11);
  int key_of_max_5 = max_submeasure(tree_);
  EXPECT_TRUE (key_of_max_5 == -5 || key_of_max_5 == -1 || key_of_max_5 == 2 || key_of_max_5 == 8);
}