/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_description_static.h"

#include "models_math.h"

#include <array>

#include <assert.h>

// protected constructors
GasParameters::GasParameters(double v,
                             double p,
                             double t,
                             const const_parameters cgp,
                             dyn_parameters dgp)
    : status_(STATUS_DEFAULT),
      sph_(state_phase::GAS),
      vpte_(parameters{v, p, t}),
      dyn_params_(dgp),
      const_params(cgp) {}

GasParameters::GasParameters(parameters prs,
                             const_parameters cgp,
                             dyn_parameters dgp)
    : status_(STATUS_DEFAULT),
      sph_(state_phase::GAS),
      vpte_(prs),
      dyn_params_(dgp),
      const_params(cgp) {}

GasParameters::~GasParameters() {}

std::ostream& operator<<(std::ostream& outstream, const GasParameters& gp) {
  char msg[256] = {0};
  parameters prs = gp.cgetParameters();
  snprintf(msg, sizeof(msg) - 1, "v: %8.2f  p: %8.2f t:%6.2f\n", prs.volume,
           prs.pressure, prs.temperature);
  outstream << msg;
  return outstream;
}

// const_gasparametrs fields
double GasParameters::cgetV_K() const {
  return const_params.critical.volume;
}

double GasParameters::cgetP_K() const {
  return const_params.critical.pressure;
}

double GasParameters::cgetT_K() const {
  return const_params.critical.temperature;
}

double GasParameters::cgetAcentricFactor() const {
  return const_params.acentricfactor;
}

double GasParameters::cgetMolecularMass() const {
  return const_params.mp.mass;
}

double GasParameters::cgetR() const {
  return const_params.mp.Rm;
}

double GasParameters::cgetCP() const {
  return dyn_params_.heat_cap_pres;
}

double GasParameters::cgetBeta() const {
  return dyn_params_.beta_kr;
}

// current parametrs of gas
double GasParameters::cgetVolume() const {
  return vpte_.volume;
}

double GasParameters::cgetPressure() const {
  return vpte_.pressure;
}

double GasParameters::cgetTemperature() const {
  return vpte_.temperature;
}

dyn_setup GasParameters::cgetDynSetup() const {
  return dyn_params_.setup;
}

double GasParameters::cgetIntEnergy() const {
  return dyn_params_.internal_energy;
}

state_phase GasParameters::cgetState() const {
  return sph_;
}

parameters GasParameters::cgetParameters() const {
  return vpte_;
}

const dyn_parameters& GasParameters::cgetDynParameters() const {
  return dyn_params_;
}

const const_parameters& GasParameters::cgetConstparameters() const {
  return const_params;
}

void GasParameters::csetParameters(double v,
                                   double p,
                                   double t,
                                   state_phase sp) {
  vpte_ = {v, p, t};
  sph_ = sp;
}

double GasParameters::cCalculateVolume(double p, double t) {
  (void)p;
  (void)t;
  return vpte_.volume;
}
