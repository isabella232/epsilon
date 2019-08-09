#include <poincare/print_float.h>
#include <poincare/ieee754.h>
#include <poincare/infinity.h>
#include <poincare/preferences.h>
#include <poincare/print_int.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>
#include <ion/unicode/utf8_decoder.h>
#include <ion/unicode/utf8_helper.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
}
#include <cmath>

namespace Poincare {

static inline int minInt(int x, int y) { return x < y ? x : y; }
static inline int maxInt(int x, int y) { return x > y ? x : y; }

PrintFloat::Long::Long(int64_t i) :
  m_negative(i < 0)
{
  int64_t nonNegativeI = m_negative ? -i : i;
  m_digits[1] = 0 + (nonNegativeI % k_base);
  m_digits[0] = (nonNegativeI - m_digits[1]) / k_base;
}

PrintFloat::Long::Long(uint32_t d1, uint32_t d2, bool negative) {
  m_digits[1] = d2;
  m_digits[0] = d1;
  m_negative = negative && !isZero();
}

void PrintFloat::Long::DivisionByTen(const Long & longToDivide, Long * quotient, Long * digit) {
  assert(quotient != nullptr && digit != nullptr);
  if (longToDivide.isZero()) {
    *quotient = Long(0);
    *digit = Long(0);
    return;
  }

  uint32_t digit0 = longToDivide.digit(0);
  uint32_t digit1 = longToDivide.digit(1);

  uint32_t d = digit1 % 10;
  uint32_t digit1DividedByTen = (digit1 - d)/10;
  *digit = Long(d);

  if (digit0 == 0) {
    *quotient = Long(0, digit1DividedByTen, longToDivide.isNegative());
  } else {
    *quotient = Long(digit0/10, digit1DividedByTen + (k_base/10) * (digit0 % 10), longToDivide.isNegative());
  }
}

void PrintFloat::Long::MultiplySmallLongByTen(Long & smallLong) {
  assert(smallLong.digit(0) == 0 && smallLong.digit(1) < (k_base/1000) && !smallLong.isZero());
  uint32_t newDigit1 = smallLong.digit(1) * 10;
  smallLong = Long(0, newDigit1, smallLong.isNegative());
}

int PrintFloat::Long::serialize(char * buffer, int bufferSize) const {
  if (bufferSize == 0) {
    return -1;
  }
  if (bufferSize == 1) {
    buffer[0] = 0;
    return 0;
  }
  int numberOfChars = m_negative ? 1 : 0; // 1 for the minus sign char
  if (m_digits[0] != 0) {
    numberOfChars += PrintInt::Left(m_digits[0], buffer + numberOfChars, bufferSize - numberOfChars - 1);
    int wantedNumberOfChars = numberOfChars + k_maxNumberOfCharsForDigit + 1;
    if (wantedNumberOfChars > bufferSize) {
      /* There is not enough space for serializing the second digit and the null
       * terminating char. */
      return wantedNumberOfChars;
    }
    numberOfChars+= PrintInt::Right(m_digits[1], buffer + numberOfChars, minInt(k_maxNumberOfCharsForDigit, bufferSize - numberOfChars - 1));
  } else {
    numberOfChars+= PrintInt::Left(m_digits[1], buffer + numberOfChars, bufferSize - numberOfChars - 1);
  }
  if (m_negative) {
    assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
    if (bufferSize > 0) {
      buffer[0] = '-';
    }
  }
  if (numberOfChars <= bufferSize - 1) {
    buffer[numberOfChars] = 0;
  }
  return numberOfChars;
}

void PrintFloat::PrintLongWithDecimalMarker(char * buffer, int bufferLength, Long & i, int decimalMarkerPosition) {
  /* The decimal marker position is always preceded by a char, thus, it is never
   * in first position. When called by ConvertFloatToText, the buffer length is
   * always > 0 as we asserted a minimal number of available chars. */
  assert(bufferLength > 0 && decimalMarkerPosition != 0);
  constexpr int tempBufferSize = PrintFloat::k_maxFloatBufferSize;
  char tempBuffer[tempBufferSize];
  int intLength = i.serialize(tempBuffer, tempBufferSize);
  int firstDigitChar = UTF8Helper::CodePointIs(tempBuffer, '-') ? 1 : 0;
  /* We should use the UTF8Decoder to write code points in buffers, but it is
   * much clearer to manipulate chars directly as we know that the code point we
   * use ('.', '0, '1', '2', ...) are only one char long. */
  for (int k = bufferLength-1; k >= firstDigitChar; k--) {
    if (k == decimalMarkerPosition) {
      assert(UTF8Decoder::CharSizeOfCodePoint('.') == 1);
      buffer[k] = '.';
      continue;
    }
    if (intLength > firstDigitChar) {
      assert(UTF8Decoder::CharSizeOfCodePoint(tempBuffer[intLength-1]) == 1);
      buffer[k] = tempBuffer[--intLength];
      continue;
    }
    assert(UTF8Decoder::CharSizeOfCodePoint('0') == 1);
    buffer[k] = '0';
  }
  if (firstDigitChar == 1) {
    assert(UTF8Decoder::CharSizeOfCodePoint(tempBuffer[0]) == 1);
    buffer[0] = tempBuffer[0];
  }
}

template <class T>
int PrintFloat::ConvertFloatToText(T f, char * buffer, int bufferSize,
    int numberOfSignificantDigits, Preferences::PrintFloatMode mode, bool allowRounding)
{
  assert(numberOfSignificantDigits > 0);
  assert(bufferSize > 0);

  /* Truncate the buffer if needed.
   * Example: 1+1.234E-30+... in decimal mode, because we do not want the fill
   * the buffer with the decimal version of 1.234E-30. */
  int truncatedBufferSize = minInt(PrintFloat::k_maxFloatBufferSize, bufferSize);

  int numberOfZerosRemoved = 0;
  int requiredLength = ConvertFloatToTextPrivate(f, buffer, truncatedBufferSize, numberOfSignificantDigits, mode, &numberOfZerosRemoved, false);
  /* If the required buffer size overflows the buffer size, we first force the
   * display mode to scientific and decrease the number of significant digits to
   * fit the buffer size. */
  if (mode == Preferences::PrintFloatMode::Decimal && requiredLength >= truncatedBufferSize) {
    constexpr int tempBufferSize = PrintFloat::k_maxFloatBufferSize;
    char tempBuffer[tempBufferSize];
    requiredLength = ConvertFloatToTextPrivate(f, tempBuffer, tempBufferSize, numberOfSignificantDigits, Preferences::PrintFloatMode::Scientific, &numberOfZerosRemoved, true);
    if (requiredLength < truncatedBufferSize) {
      return strlcpy(buffer, tempBuffer, truncatedBufferSize);
    }
  }
  if (requiredLength >= truncatedBufferSize) {
    /* If the buffer size is still too small and rounding is allowed, we only
     * write the beginning of the float and truncate it (which can result in a
     * non sense text). If no rounding is allowed, we set the text to null. */
    if (!allowRounding) {
      buffer[0] = 0;
      return 0;
    }
    int adjustedNumberOfSignificantDigits = numberOfSignificantDigits - numberOfZerosRemoved - (requiredLength - (truncatedBufferSize - 1));
    if (adjustedNumberOfSignificantDigits < 1) {
      adjustedNumberOfSignificantDigits = 1;
    }
    requiredLength = ConvertFloatToTextPrivate(f, buffer, truncatedBufferSize, adjustedNumberOfSignificantDigits, Preferences::PrintFloatMode::Scientific, &numberOfZerosRemoved, false);
  }
  if (requiredLength >= truncatedBufferSize) {
    buffer[0] = 0;
    return 0;
  }
  return requiredLength;
}

static int engineeringExponentFromExponentBase10(int exponentInBase10) {
  int exponentInBase10Mod3 = exponentInBase10 % 3;
  return exponentInBase10 - (exponentInBase10Mod3 + (exponentInBase10Mod3 < 0 ? 3 : 0));
}

template <class T>
int PrintFloat::ConvertFloatToTextPrivate(T f, char * buffer, int bufferSize, int numberOfSignificantDigits, Preferences::PrintFloatMode mode, int * numberOfRemovedZeros, bool returnTrueRequiredLength) {
  assert(numberOfSignificantDigits > 0);
  assert(bufferSize > 0);
  //TODO: accelerate for f between 0 and 10 ?
  if (std::isinf(f)) {
    // Infinity
    bool writeMinusSign = f < 0;
    assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
    int requiredLength = (writeMinusSign ? 1 : 0) + Infinity::NameSize() - 1;
    if (requiredLength > bufferSize - 1) {
      // We will not be able to print
      return requiredLength;
    }
    // Write inf or -inf
    int currentChar = 0;
    if (f < 0) {
      currentChar+= SerializationHelper::CodePoint(buffer, bufferSize, '-');
    }
    assert(bufferSize - currentChar > 0);
    strlcpy(buffer+currentChar, Infinity::Name(), bufferSize-currentChar);
    return requiredLength;
  }

  if (std::isnan(f)) {
    // Nan
    constexpr int requiredLength = Undefined::NameSize() - 1;
    if (requiredLength - 1 > bufferSize - 1) {
      // We will not be able to print
      return requiredLength;
    }
    return strlcpy(buffer, Undefined::Name(), bufferSize);
  }

  int exponentInBase10 = IEEE754<T>::exponentBase10(f);
  int exponentInEngineeringNotation = engineeringExponentFromExponentBase10(exponentInBase10);
  assert(exponentInEngineeringNotation%3 == 0);
  assert(exponentInEngineeringNotation <= exponentInBase10 && exponentInBase10 < exponentInEngineeringNotation + 3);

  /* Part I: Mantissa */

  // Compute mantissa
  T unroundedMantissa = f * std::pow((T)10.0, (T)(numberOfSignificantDigits - 1 - exponentInBase10));
  // Round mantissa to get the right number of significant digits
  T mantissa = std::round(unroundedMantissa);

  /* If (numberOfSignificantDigits - 1 - exponentInBase10) is too big (or too
   * small), mantissa is now inf. We handle this case by using logarithm
   * function. */
  if (std::isnan(mantissa) || std::isinf(mantissa)) {
    mantissa = std::round(std::pow(10, std::log10(std::fabs(f))+(T)(numberOfSignificantDigits -1 - exponentInBase10)));
    mantissa = std::copysign(mantissa, f);
  }
  /* We update the exponent in base 10 (if 0.99999999 was rounded to 1 for
   * instance)
   * NB: the following if-condition should rather be:
   * "exponentBase10(unroundedMantissa) != exponentBase10(mantissa)",
   * However, unroundedMantissa can have a different exponent than expected
   * (ex: f = 1E13, unroundedMantissa = 99999999.99 and mantissa = 1000000000) */
  if (f != 0 && IEEE754<T>::exponentBase10(mantissa) - exponentInBase10 != numberOfSignificantDigits - 1 - exponentInBase10) {
    exponentInBase10++;
    exponentInEngineeringNotation = engineeringExponentFromExponentBase10(exponentInBase10);
  }

  if (mode == Preferences::PrintFloatMode::Decimal && exponentInBase10 >= numberOfSignificantDigits) {
    /* Exception 1: avoid inventing digits to fill the printed float: when
     * displaying 12345 with 2 significant digis in Decimal mode for instance.
     * This exception is caught by ConvertFloatToText and forces the mode to
     * Scientific */
    assert(!returnTrueRequiredLength);
    return bufferSize + 1;
  }

  // Correct the number of digits in mantissa after rounding
  if (IEEE754<T>::exponentBase10(mantissa) >= numberOfSignificantDigits) {
    mantissa = mantissa/10.0;
  }

  // Number of chars for the mantissa
  int numberOfCharsForMantissaWithoutSign = 0;
  if (mode == Preferences::PrintFloatMode::Decimal) {
    if (exponentInBase10 >= 0) {
      numberOfCharsForMantissaWithoutSign = numberOfSignificantDigits;
    } else {
      numberOfCharsForMantissaWithoutSign = numberOfSignificantDigits - exponentInBase10;
    }
  } else if (mode == Preferences::PrintFloatMode::Scientific) {
    numberOfCharsForMantissaWithoutSign = numberOfSignificantDigits;
  } else {
    assert(mode == Preferences::PrintFloatMode::Engineering);
    numberOfCharsForMantissaWithoutSign = numberOfSignificantDigits + maxInt(0, (numberOfSignificantDigits%3) - numberOfSignificantDigits + 1);
    assert(numberOfCharsForMantissaWithoutSign <= numberOfSignificantDigits);
  }

  /* The number of digits in a mantissa is capped because the maximal int64_t is
   * 2^63 - 1. As our mantissa is an integer built from an int64_t, we assert
   * that we stay beyond this threshold during computation. */
  assert(numberOfSignificantDigits < std::log10(std::pow(2.0f, 63.0f)));

  // Remove/Add the zeroes on the right side of the mantissa
  Long dividend = Long((int64_t)mantissa);
  int numberOfZeroesToAddForEngineering = (numberOfSignificantDigits%3) - numberOfSignificantDigits + 1;
  int minimalNumbersOfMantissaDigitsForEngineering = exponentInBase10 - exponentInEngineeringNotation + 1;
  assert(mode != Preferences::PrintFloatMode::Engineering || (minimalNumbersOfMantissaDigitsForEngineering > 0 && minimalNumbersOfMantissaDigitsForEngineering <= 3));
  if (mode == Preferences::PrintFloatMode::Engineering && numberOfZeroesToAddForEngineering > 0) {
    int numberOfZeroesToAdd = (numberOfSignificantDigits%3) - numberOfSignificantDigits + 1;
    if (numberOfZeroesToAdd > 0) {
      assert(numberOfCharsForMantissaWithoutSign - numberOfSignificantDigits < 3);
      for (int i = 0; i <numberOfZeroesToAdd; i++) {
        assert(mantissa < 1000);
        Long::MultiplySmallLongByTen(dividend);
      }
      if (numberOfRemovedZeros != nullptr) {
        *numberOfRemovedZeros = -numberOfZeroesToAdd;
      }
    }
  } else {
    Long digit;
    Long quotient;
    Long::DivisionByTen(dividend, &quotient, &digit);
    int minimumNumberOfCharsInMantissa = mode == Preferences::PrintFloatMode::Engineering ? minimalNumbersOfMantissaDigitsForEngineering : 1;
    int numberOfZerosRemoved = 0;
    while (digit.isZero()
        && numberOfCharsForMantissaWithoutSign > minimumNumberOfCharsInMantissa
        && (numberOfCharsForMantissaWithoutSign > exponentInBase10 + 1
          || mode == Preferences::PrintFloatMode::Scientific
          || mode == Preferences::PrintFloatMode::Engineering))
    {
      assert(UTF8Decoder::CharSizeOfCodePoint('0') == 1);
      numberOfCharsForMantissaWithoutSign--;
      dividend = quotient;
      Long::DivisionByTen(dividend, &quotient, &digit);
      numberOfZerosRemoved++;
    }
    if (!returnTrueRequiredLength && numberOfCharsForMantissaWithoutSign > bufferSize - 1) {
      // Escape now if the true number of needed digits is not required
      return bufferSize + 1;
    }
    if (numberOfRemovedZeros != nullptr) {
      *numberOfRemovedZeros = numberOfZerosRemoved;
    }
  }

  /* Part II: Decimal marker */

  // Force a decimal marker if there is fractional part
  bool decimalMarker = (mode == Preferences::PrintFloatMode::Scientific && numberOfCharsForMantissaWithoutSign > 1)
    || (mode == Preferences::PrintFloatMode::Decimal && numberOfCharsForMantissaWithoutSign > exponentInBase10 + 1)
    || (mode == Preferences::PrintFloatMode::Engineering && (numberOfCharsForMantissaWithoutSign > minimalNumbersOfMantissaDigitsForEngineering));
  if (decimalMarker) {
    assert(UTF8Decoder::CharSizeOfCodePoint('.') == 1);
    numberOfCharsForMantissaWithoutSign++;
  }
  if (!returnTrueRequiredLength && numberOfCharsForMantissaWithoutSign > bufferSize - 1) {
    // Escape now if the true number of needed digits is not required
    return bufferSize + 1;
  }

  /* Find the position of the decimal marker position */
  int decimalMarkerPosition = 0;
  if (mode == Preferences::PrintFloatMode::Decimal) {
    decimalMarkerPosition = (exponentInBase10 < 0) ? 1 : exponentInBase10 + 1;
  } else if (mode == Preferences::PrintFloatMode::Scientific) {
    decimalMarkerPosition = 1;
  } else {
    assert(mode == Preferences::PrintFloatMode::Engineering);
    decimalMarkerPosition = minimalNumbersOfMantissaDigitsForEngineering;
  }
  if (f < 0) {
    decimalMarkerPosition++;
  }
  assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);

  /* Part III: Sign */

  int numberOfCharsForMantissaWithSign = numberOfCharsForMantissaWithoutSign + (f >= 0 ? 0 : 1);
  if (numberOfCharsForMantissaWithSign > bufferSize - 1) {
    // Exception 2: we will overflow the buffer
    assert(mode == Preferences::PrintFloatMode::Decimal);
    assert(!returnTrueRequiredLength);
    return bufferSize + 1;
  }

  /* Part IV: Exponent */

  int exponent = mode == Preferences::PrintFloatMode::Engineering ? exponentInEngineeringNotation : exponentInBase10;
  int numberOfCharExponent = exponent != 0 ? std::log10(std::fabs((T)exponent)) + 1 : 0;
  if (exponent < 0) {
    // If the exponent is < 0, we need a additional char for the sign
    numberOfCharExponent++;
    assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
  }

  /* Part V: print mantissa*10^exponent */

  assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
  // Print mantissa
  bool doNotWriteExponent = (mode == Preferences::PrintFloatMode::Decimal) || (exponent == 0);
  int neededNumberOfChars = numberOfCharsForMantissaWithSign + (doNotWriteExponent ? 0 : UTF8Decoder::CharSizeOfCodePoint(UCodePointLatinLetterSmallCapitalE) + numberOfCharExponent);
  if (neededNumberOfChars > bufferSize - 1) {
    // Exception 3: We are about to overflow the buffer.
    return neededNumberOfChars;
  }
  PrintLongWithDecimalMarker(buffer, numberOfCharsForMantissaWithSign, dividend, decimalMarkerPosition);
  if (doNotWriteExponent) {
    buffer[numberOfCharsForMantissaWithSign] = 0;
    return numberOfCharsForMantissaWithSign;
  }
  // Print exponent
  assert(numberOfCharsForMantissaWithSign < bufferSize);
  int currentNumberOfChar = numberOfCharsForMantissaWithSign;
  currentNumberOfChar+= UTF8Decoder::CodePointToChars(UCodePointLatinLetterSmallCapitalE, buffer + currentNumberOfChar, bufferSize - currentNumberOfChar);
  dividend = Long(exponent); // reuse dividend as it is not needed anymore
  PrintLongWithDecimalMarker(buffer + currentNumberOfChar, numberOfCharExponent, dividend, -1);
  buffer[currentNumberOfChar + numberOfCharExponent] = 0;
  assert(neededNumberOfChars == currentNumberOfChar + numberOfCharExponent);
  return currentNumberOfChar + numberOfCharExponent;
}

template int PrintFloat::ConvertFloatToText<float>(float, char*, int, int, Preferences::Preferences::PrintFloatMode, bool);
template int PrintFloat::ConvertFloatToText<double>(double, char*, int, int, Preferences::Preferences::PrintFloatMode, bool);

}
