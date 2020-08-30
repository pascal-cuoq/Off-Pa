#include <stdlib.h>

int rand(void) {
  static const int random_sequence[] = {
    123, 31983, 9827, 15830, 9257, 4951, 7111, 19876,
    8723, 98, 4929, 29999, 13648, 1111, 743, 3839,
    4520, 18866, 23457, 1067, 3589, 13849, 999,
    17434, 9383, 5701, 17, 23953, 22368, 8235,
    21828, 27988, 6265,
  };
  static unsigned index = 0;
  int result = random_sequence[index];
  index++;
  // This function is intended to use in TIS-CI only,
  // TIS-CI will gently warn if we go out of range here
  // so that we can add more random numbers
  return result;
}
