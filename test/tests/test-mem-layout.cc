#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cmath>

#include <UnitTesting.h>
#include "marlin/book/Types.h"
#include "marlin/book/MemLayout.h"

using namespace marlin::book;
using namespace marlin::book::types;

struct Type1 {
  std::array<int, 5> bins;
  Type1(int a0, int a1, int a2, int a3, int a4)
    : bins{a0, a1, a2, a3, a4} {}
};

void MergeType1(const std::shared_ptr<Type1>& p1, const std::shared_ptr<Type1>& p2) {
  for(int i = 0; i < 5; ++i) {
    p1->bins[i] += p2->bins[i];
  }
}

int main (int, char**) {
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  marlin::test::UnitTest test(" MemLayout Test ");
  SharedMemLayout<Type1, MergeType1, int, int, int, int, int> sMem(3, 1, 0, 0, 0, 0);
  auto ptr1 = sMem.at<Type1>(0);
  auto ptr2 = sMem.at<Type1>(1);
  ptr1->bins[0] += 3;
  ptr2->bins[0] += 2;
  test.test("bin content test ", sMem.merged<Type1>()->bins[0] == 9);

  SharedMemLayout<H1I, add<HistConfig<double, int, 1>>, AxisConfig<double>> sMemH(3, {"x", 2, 0, RAND_MAX});
  H1D refHist(AxisConfig<double>("x", 2, 0, RAND_MAX));
  auto ptrH1 = sMemH.at<H1I>(0);
  auto ptrH2 = sMemH.at<H1I>(1);

  for(int i = 0; i < 10; ++i) {
    double x
      = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    int w = std::rand() % 100;

    if(std::rand() > RAND_MAX / 2) {
      ptrH1->Fill({x}, w);
      std::cout << ptrH1->get().GetBinContent({x});
    } else {
      ptrH2->Fill({x}, w);
      std::cout << ptrH2->get().GetBinContent({x});
    }
    refHist.Fill({x}, w);
  }

  test.test("Test shared H1D", 
    refHist.get().GetBinContent({0}) == sMemH.merged<H1I>()->get().GetBinContent({0})
  ) ;
  return 0;
}

