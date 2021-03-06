#include "Color.h"
#include <sstream>
#include <string>

uint8_t alpha_over_c(uint8_t u, uint8_t o, uint8_t ua, uint8_t oa);

/**
 * Takes two color values and does an alpha over blending without using floats.
 */
inline uint8_t alpha_over_c(uint8_t ac, uint8_t aa, uint8_t bc, uint8_t ba) {
  uint8_t caa = (ac * aa) / 0xff;
  uint8_t cba = (bc * ba) / 0xff;
  return (caa + ((0xff - aa) * cba) / 0xff);
}

void Color::overlay(const Color &other) {
  r = alpha_over_c(other.r, other.a, r, a);
  g = alpha_over_c(other.g, other.a, g, a);
  b = alpha_over_c(other.b, other.a, b, a);
  a = a + (other.a * (0xff - a)) / 0xff;
}

void Color::underlay(const Color &other) {
  r = alpha_over_c(r, a, other.r, other.a);
  g = alpha_over_c(g, a, other.g, other.a);
  b = alpha_over_c(b, a, other.b, other.a);
  a = other.a + (a * (0xff - other.a)) / 0xff;
}

void Color::blend(const Color &other) {
  if (other.is_transparent()) return;
  r = alpha_over_c(other.r, other.a, r, a);
  g = alpha_over_c(other.g, other.a, g, a);
  b = alpha_over_c(other.b, other.a, b, a);
  a = a + (other.a * (0xff - a)) / 0xff;
  r = ((r * 0xff) / a);
  g = ((g * 0xff) / a);
  b = ((b * 0xff) / a);
}

// pull color down towards black
void Color::darken(uint8_t f) {
  r -= ((r * f) / 0xff);
  g -= ((g * f) / 0xff);
  b -= ((b * f) / 0xff);
}
