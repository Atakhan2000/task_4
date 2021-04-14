#include "hash.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct TestStruct
{
    std::string key;
    std::string value1;
    std::string value2;

    TestStruct()
    {
    };

    ~TestStruct()
    {
    };

    TestStruct(const TestStruct& struct_)
    {
        key = struct_.key;
        value1 = struct_.value1;
        value2 = struct_.value2;
    }
    bool operator==(const TestStruct& t) const
    {
        return (key == t.key) && (value1 == t.value1) && (value2 == t.value2);
    }

};


std::string randomString(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length,0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

static void generate(TestStruct *pts)
{
    pts->key = randomString(10);
    pts->value1 = randomString(20);
    pts->value2 = randomString(20);
}

unsigned int HashFunc(const TestStruct* pElement )
{
  const int p = 31;
  unsigned int hash = 0;
  size_t p_pow = 1;

  for (char i: pElement->key)
  {
    hash += (i-'A') * p_pow;
    p_pow *= p;
  }
  return hash;
}

int Compare (const TestStruct* pElement1, const TestStruct* pElement2)
{
  if (pElement1->key == pElement2->key)
  {
    return 0;
  }

  if (pElement1->key < pElement2->key)
  {
    return 1;
  }
  else
  {
    return -1;
  }
}
void TestAdd(TestStruct* array,lab618::CHash<TestStruct, HashFunc, Compare>& Table, int& n)
  {
    for (int i = 0; i < n; ++i)
    {
      Table.add(&array[i]);
    }

    for (int i = 0; i < n; ++i)
    {
      auto pT = Table.find(array[i]);
      assert(*pT == array[i]);
    }
  }

void TestRemove(TestStruct* array,lab618::CHash<TestStruct, HashFunc, Compare>& Table, int& n)
  {
    for (int i = 0; i < n; ++i)
    {
      auto pT = Table.find(array[i]);
      Table.remove(*pT);
    }

    for (int i = 0; i < n; ++i)
    {
      auto pT = Table.find(array[i]);
      assert(pT == nullptr);
    }
  }

int main() {
  int n = 100;
  TestStruct* array = new TestStruct[n];
  lab618::CHash<TestStruct, HashFunc, Compare> Table(31, 2);
  for (int i = 0; i < n; ++i)
  {
    generate(&(array[i]));
  }

  TestAdd(array, Table, n);
  TestRemove(array, Table, n);

  return 0;
}

