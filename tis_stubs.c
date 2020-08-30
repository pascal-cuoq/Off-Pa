#include <stdlib.h>

int rand(void) {
  static const int random_sequence[] = {
    123, 31983, 9827, 15830, 9257, 4951, 7111, 19876,
  };
  static unsigned index = 0;
  int result = random_sequence[index];
  index++;
  // This function is intended to use in TIS-CI only,
  // TIS-CI will gently warn if we go out of range here
  // so that we can add more random numbers
  return result;
}
