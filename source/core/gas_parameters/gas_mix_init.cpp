#include "gas_mix_init.h"

#include "common.h"
#include "models_errors.h"
#include "model_general.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <assert.h>
#include <string.h>

gas_mix_file::gas_mix_file(const std::string filename, const double part) 
  : filename(filename), part(part) {}

bool operator< (const gas_mix_file &lg, const gas_mix_file &rg) {
  return strcmp(lg.filename.c_str(), rg.filename.c_str()) <= 0;
}

// implicit functions
static std::array<double, 5> get_average_params(
    const parameters_mix &components) {
  // acentric = 1.0, тому что считается через логарифм
  // и следовательно вычисляем как среднее ГЕОМЕТРИЧЕСКОЕ
  //   в отличии от других параметров
  //   вообще, просто по интуиции, без подоплеки
  std::array<double, 5> avr_vals = {0.0, 0.0, 0.0, 0.0, 1.0};
  for (auto const &x : components) {
    avr_vals[0] += x.first * x.second.first.V_K;
    avr_vals[1] += x.first * x.second.first.P_K;
    avr_vals[2] += x.first * x.second.first.T_K;

    avr_vals[3] += x.first * x.second.first.molecularmass;
    avr_vals[4] *= x.first * x.second.first.acentricfactor;
  }
  avr_vals[4] = std::pow(avr_vals[4], 1.0 / components.size());
  return avr_vals;
}

// GasParameters_mix
GasParameters_mix::GasParameters_mix(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components)
  : GasParameters(prs, cgp, dgp), components_(components) {}

 GasParameters_mix::~GasParameters_mix() {}

GasParameters_mix_dyn::GasParameters_mix_dyn(parameters prs,
    const_parameters cgp, dyn_parameters dgp, parameters_mix components,
    modelGeneral *mg)
  : GasParameters_mix(prs, cgp, dgp, components), model_(mg) {}

GasParameters_mix_dyn *GasParameters_mix_dyn::Init(
    gas_params_input gpi, modelGeneral *mg) {
  if (gpi.const_dyn.components->empty() || mg == nullptr) {
    set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST | ERR_GAS_MIX);
    return nullptr;
  }
  reset_error();
  std::array<double, 5> avr_vals = get_average_params(
      *gpi.const_dyn.components);
  // init gasmix const_parameters
  std::unique_ptr<const_parameters> tmp_cgp(const_parameters::Init(
      avr_vals[0], avr_vals[1], avr_vals[2], avr_vals[3], avr_vals[4]));
  if (tmp_cgp == nullptr) {
    set_error_code(ERR_INIT_T | ERR_GAS_MIX | ERR_CALC_GAS_P_ST);
    return nullptr;
  }
  double volume = 0.0;
  for (const auto &x : *gpi.const_dyn.components)
    volume += x.first * mg->InitVolume(gpi.p, gpi.t, x.second.first);
  std::vector<std::pair<double, dyn_parameters>> dgp_cpt;
  for (auto const &x : *gpi.const_dyn.components) {
    dgp_cpt.push_back({x.first, x.second.second});
    mg->update_dyn_params(dgp_cpt.back().second,
        { volume, gpi.p, gpi.t}, x.second.first);
  }
  std::array<double, 3> dgp_tmp = {0.0, 0.0, 0.0};
  for (auto const &x : dgp_cpt) {
    dgp_tmp[0] += x.first * x.second.heat_cap_vol;
    dgp_tmp[1] += x.first * x.second.heat_cap_pres;
    dgp_tmp[2] += x.first * x.second.internal_energy;
  }
  std::unique_ptr<dyn_parameters> tmp_dgp(dyn_parameters::Init(
      dgp_tmp[0], dgp_tmp[1], dgp_tmp[2], {volume, gpi.p, gpi.t}));
  if (tmp_dgp == nullptr) {
    set_error_code(ERR_INIT_T | ERR_CALC_GAS_P_ST | ERR_GAS_MIX);
    return nullptr;
  }
  return new GasParameters_mix_dyn({0.0, gpi.p, gpi.t}, *tmp_cgp, *tmp_dgp,
      *gpi.const_dyn.components, mg);
}

std::unique_ptr<const_parameters> GasParameters_mix_dyn::GetAverageParams(
    parameters_mix &components) {
  if (components.empty()) {
    set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST | ERR_GAS_MIX);
    return nullptr;
  }
  reset_error();
  std::array<double, 5> avr_vals = get_average_params(components);
  // init gasmix const_parameters
  std::unique_ptr<const_parameters> tmp_cgp(const_parameters::Init(
      avr_vals[0], avr_vals[1], avr_vals[2], avr_vals[3], avr_vals[4]));
  if (tmp_cgp == nullptr) {
    set_error_code(ERR_INIT_T | ERR_GAS_MIX | ERR_CALC_GAS_P_ST);
    return nullptr;
  }
  return tmp_cgp;
}

const parameters_mix &GasParameters_mix_dyn::GetComponents() const {
  return components_;
}

#ifndef GAS_MIX_VARIANT
void GasParameters_mix_dyn::csetParameters(double v, double p, double t,
    state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume       = v;
  vpte_.pressure     = p;
  vpte_.temperature  = t;
  sph_ = sp;
  for (auto &x : components_)
    model_->update_dyn_params(x.second.second, vpte_);
  dyn_params_.heat_cap_vol  = 0.0;
  dyn_params_.heat_cap_pres = 0.0;
  dyn_params_.internal_energy = 0.0;
  dyn_params_.beta_kr = 0.0;
  dyn_params_.parm = vpte_;
  for (auto const &x : components_) {
    dyn_params_.heat_cap_vol  += x.first * x.second.second.heat_cap_vol;
    dyn_params_.heat_cap_pres += x.first * x.second.second.heat_cap_pres;
    dyn_params_.internal_energy +=
        x.first * x.second.second.internal_energy;
    dyn_params_.beta_kr += x.first * x.second.second.beta_kr;
  }
}
#else
void GasParameters_mix_dyn::csetParameters(double v, double p, double t,
    state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume       = v;
  vpte_.pressure     = p;
  vpte_.temperature  = t;
  sph_ = sp;
  for (auto &x : components_)
    model_->update_dyn_params(x.second.second, vpte_, x.second.first);
  dyn_params_.heat_cap_vol  = 0.0;
  dyn_params_.heat_cap_pres = 0.0;
  dyn_params_.internal_energy = 0.0;
  dyn_params_.beta_kr = 0.0;
  dyn_params_.parm = vpte_;
  for (auto const &x : components_) {
    dyn_params_.heat_cap_vol  += x.first * x.second.second.heat_cap_vol;
    dyn_params_.heat_cap_pres += x.first * x.second.second.heat_cap_pres;
    dyn_params_.internal_energy +=
        x.first * x.second.second.internal_energy;
    dyn_params_.beta_kr += x.first * x.second.second.beta_kr;
  }
}
#endif  // !GAS_MIX_VARIANT
