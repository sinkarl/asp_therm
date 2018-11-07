#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_

#include "gas_description.h"
// Данные из ГОСТ 30319.3_2015

// data structs
struct component_characteristics {
  const gas_t gas_name;
  const double M,  // molar_mass
               Z,  // compress_coef - для НФУ
               E,  // energy_param
               K,  // size_param
               G,  // orintation_param
               Q,  // quadrup_param
               F,  // high_temp_param
               S,  // gipol_param
               W;  // associate_param
};
const component_characteristics *get_characteristics(gas_t gas_name);

struct binary_associate_coef {
  const gas_t i,
              j;
  const double E,
               V,
               K,
               G;
};
const binary_associate_coef *get_binary_associate_coefs(gas_t i, gas_t j);

extern const size_t A0_3_coefs_count;
struct A0_3_coef {
  const double a,
               b,
               c,
               k,
               u,
               g,
               q,
               t,
               s,
               w;
};

struct A4_coef {
  const gas_t gas_name;
  const double B,
               C,
               D,
               E,
               F,
               G,
               H,
               I,
               J;
};
const A4_coef *get_A4_coefs(gas_t gas_name);

struct critical_params {
  const gas_t gas_name;
  const double temperature,
               density,
               acentric;
};
const critical_params *get_critical_params(gas_t gas_name);

struct A6_coef {
  const gas_t gas_name;
  const double k0,
               k1,
               k2,
               k3;
};
const A6_coef *get_A6_coefs(gas_t gas_name);

struct A7_coef {
  const double c;
  const int r,
            t;
};

struct A8_coef {
  const gas_t gas_name;
  const double k1,
               k2,
               k3,
               k4,
               k5,
               k6;
};
const A8_coef *get_A8_coefs(gas_t gas_name);

struct A9_molar_mass {
  const gas_t gas_name;
  const double M;  // molar_mass
};
const A9_molar_mass *get_molar_mass(gas_t gas_name);

// data
extern const component_characteristics gases[];
extern const binary_associate_coef gases_coef[];
extern const A0_3_coef A0_3_coefs[];
extern const A4_coef A4_coefs[];
extern const critical_params A5_critical_params[];
extern const A6_coef A6_coefs[];
extern const A7_coef A7_coefs[];
extern const A8_coef A8_coefs[];
extern const int A8_sigmas[];
extern const A9_molar_mass A9_molar_masses[];

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_