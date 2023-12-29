#include <stdio.h>

void printFloat(char *bytes);
unsigned int getBit(char *bytes, int pos);

unsigned int getExponent(char *f);
void printExponent(unsigned int exponent);
unsigned int getSignificand(char *f);

unsigned int prepareSignificand(unsigned int rawSignificand);
void printPreparedSignificand(unsigned int preparedSignificand,
                              int printDecimalPoint);
void printIntermediateRepresentation(unsigned int sign,
                                     unsigned int significand,
                                     unsigned int exponent,
                                     int printActualBits);

int main(void) {
  float x, y;
  char *f1, *f2;

  unsigned int sign1, sign2;

  int exponentDifference;
  unsigned int exponent1, exponent2;
  unsigned int resultExponent;
  unsigned int preparedSignificand1, preparedSignificand2;
  unsigned int additiveSignificand1, additiveSignificand2;

  unsigned int resultSignBit;
  unsigned int largerSignificand, smallerSignificand;

  unsigned int resultSignificandAndGuard;
  char *resultSignificandAndGuardBuffer;
  unsigned int resultSignificand;
  unsigned int guard;

  unsigned int resultInt;
  float result;

  scanf("%f %f", &x, &y);

  f1 = (char *)&x;
  f2 = (char *)&y;

  sign1 = getBit(f1, 31);
  sign2 = getBit(f2, 31);

  printf("Num1 actual memory view: ");
  printFloat(f1);
  putchar('\n');
  printf("Num2 actual memory view: ");
  printFloat(f2);
  putchar('\n');

  exponent1 = getExponent(f1);
  exponent2 = getExponent(f2);

  resultExponent = exponent1 > exponent2 ? exponent1 : exponent2;

  exponentDifference = exponent1 - exponent2;
  preparedSignificand1 = prepareSignificand(getSignificand(f1));
  preparedSignificand2 = prepareSignificand(getSignificand(f2));

  printf(
      "\nAdding 01 (the one before the decimal point) to the front\n"
      "Adding 3 guard bits (guard, round and sticky bit) at the end...\n");
  printf("Num1: ");
  printIntermediateRepresentation(sign1, preparedSignificand1, exponent1, 1);
  putchar('\n');
  printf("Num2: ");
  printIntermediateRepresentation(sign2, preparedSignificand2, exponent2, 1);
  putchar('\n');

  if (exponentDifference < 0) {
    exponentDifference *= -1;
    additiveSignificand1 = exponentDifference < 32
                               ? preparedSignificand1 >> exponentDifference
                               : 0;
    additiveSignificand2 = preparedSignificand2;
  } else {
    additiveSignificand1 = preparedSignificand1;
    additiveSignificand2 = exponentDifference < 32
                               ? preparedSignificand2 >> exponentDifference
                               : 0;
  }

  if (exponentDifference != 0) {
    printf(
        "\nThere is an exponent difference of %d\nSo shifting the smaller "
        "exponent number(Num%d) by %d bits to the right and so also increasing "
        "the exponent by %d to balance it\n",
        exponentDifference, exponent1 < exponent2 ? 1 : 2, exponentDifference,
        exponentDifference);

    printf("Num1: ");
    printIntermediateRepresentation(sign1, additiveSignificand1, resultExponent,
                                    1);
    putchar('\n');
    printf("Num2: ");
    printIntermediateRepresentation(sign2, additiveSignificand2, resultExponent,
                                    1);
    putchar('\n');
  } else {
    printf(
        "\nThere is no shifting required since the exponents are the same\n");
  }

  printf("\nNow adding the significants will suffice\n");

  resultSignBit =
      (sign1 && sign2) ||
      (!sign1 && sign2 && additiveSignificand1 < additiveSignificand2) ||
      (sign1 && !sign2 && additiveSignificand1 > additiveSignificand2);

  largerSignificand = additiveSignificand1 > additiveSignificand2
                          ? additiveSignificand1
                          : additiveSignificand2;
  smallerSignificand = additiveSignificand1 > additiveSignificand2
                           ? additiveSignificand2
                           : additiveSignificand1;

  resultSignificandAndGuard = x * y > 0
                                  ? largerSignificand + smallerSignificand
                                  : largerSignificand - smallerSignificand;

  printf("\nAdded result: ");
  printIntermediateRepresentation(resultSignBit, resultSignificandAndGuard,
                                  resultExponent, 1);
  putchar('\n');

  resultSignificandAndGuardBuffer = (char *)&resultSignificandAndGuard;

  if (getBit(resultSignificandAndGuardBuffer, 27)) {
    resultSignificandAndGuard >>= 1;
    ++resultExponent;

    printf(
        "\nResult is not normalized (has an extra integer place before the "
        "decimal).\nSo shifting 1 bit to the right and increasing the "
        "exponent to balance it...\n");

  } else if (getBit(resultSignificandAndGuardBuffer, 26) == 0) {
    int i = 25;
    while (getBit(resultSignificandAndGuardBuffer, i) == 0 && i >= 0) --i;
    if (i >= 0) {
      resultSignificandAndGuard <<= 26 - i;
      resultExponent -= 26 - i;

      printf(
          "\nThe first 1 is found at the %dth decimal place. So shifting that "
          "much to the left and decreasing the exponent to balance it...\n",
          26 - i);

    } else {
      resultSignificandAndGuard = resultExponent = 0;
      printf("\nResult is 0, so falling back to 0's case\n");
    }
  } else {
    printf("\nResult is already normalized\n");
  }

  printf("\nNormalized result: ");
  printIntermediateRepresentation(resultSignBit, resultSignificandAndGuard,
                                  resultExponent, 1);
  putchar('\n');

  resultSignificandAndGuardBuffer = (char *)&resultSignificandAndGuard;

  resultSignificand = ((resultSignificandAndGuard >> 3) << 9) >> 9;
  guard = resultSignificandAndGuard & 7;

  {
    int lsb = getBit(resultSignificandAndGuardBuffer, 3);
    if ((guard == 4 && lsb) || guard > 4) {
      if (guard > 4)
        printf("\nGuard bits are more than 100, So rounding by adding 1...\n");
      else
        printf(
            "\nGuard bits are 100, and LSB is odd, So rounding to even...\n");

      ++resultSignificand;
      resultSignificand = (resultSignificand << 9) >> 9;
      if (resultSignificand == 0) {
        printf(
            "But that denormalizes the number, so right shifting and also "
            "increasing the exponent to balance it\n");
        ++resultExponent;
      }
    } else {
      if (guard < 4)
        printf(
            "\nGuard bits are less than 100, So rounding by truncating...\n");
      else
        printf(
            "Guard bits are 100 and LSB is even, so rounding by truncating...");
    }
  }

  resultInt =
      (resultSignBit << 31) | (resultExponent << 23) | resultSignificand;

  result = *((float *)&resultInt);

  printf("\nFinal Result: ");
  printFloat((char *)&result);

  printf("\n\t= %f\n", result);
  printf("\nResult by invoking the actual hardware: %f\n\n", x + y);

  return 0;
}

void printFloat(char *bytes) {
  int i;
  for (i = 31; i >= 0; --i) {
    printf("%d", getBit(bytes, i));
    if (i == 31 || i == 23) printf(" ");
  }
}

unsigned int getBit(char *bytes, int pos) {
  return (1 << (pos % 8)) & bytes[pos / 8] ? 1u : 0u;
}

unsigned int getExponent(char *f) {
  unsigned int number = 0;
  int i;
  for (i = 23; i <= 30; ++i) {
    unsigned int bit = getBit(f, i);
    number = (bit << (i - 23)) + number;
  }
  return number;
}

unsigned int getSignificand(char *f) {
  unsigned int number = 0;
  int i;
  for (i = 22; i >= 0; --i) {
    unsigned int bit = getBit(f, i);
    number = (number << 1) + bit;
  }
  return number;
}

unsigned int prepareSignificand(unsigned int rawSignificand) {
  return (1 << 26) | (rawSignificand << 3);

  /*
    00000 1 rawSignificand(23 bits) 000
  */
}

void printPreparedSignificand(unsigned int preparedSignificand,
                              int printDecimalPoint) {
  char *bytes = (char *)&preparedSignificand;
  int i;
  for (i = 27; i >= 0; --i) {
    int bit = getBit(bytes, i);
    printf("%d", bit);
    if (i == 26) putchar(printDecimalPoint ? '.' : ' ');
    if (!printDecimalPoint && i == 3) putchar(' ');
  }
}

void printExponent(unsigned int exponent) {
  char *exponentBuffer = (char *)&exponent;
  int i;
  for (i = 7; i >= 0; --i) {
    printf("%d", getBit(exponentBuffer, i));
  }
}

void printIntermediateRepresentation(unsigned int sign,
                                     unsigned int significand,
                                     unsigned int exponent,
                                     int printActualBits) {
  if (printActualBits) {
    printf("\n\tMemory view: ");
    printf("%d ", sign != 0);
    printExponent(exponent);
    putchar(' ');
    printPreparedSignificand(significand, 0);
    printf("\n\tNumber view: ");
  }

  if (sign == 1) printf("-");
  printPreparedSignificand(significand, 1);
  printf(" X 2^%d", exponent - 127);
}