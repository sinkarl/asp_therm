/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gasmix_init.h"

#include "common.h"
#include "ErrorWrap.h"
#include "model_general.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <assert.h>
#include <string.h>

gasmix_file::gasmix_file(const std::string &name, const std::string &path,
    const double part): name(name), path(path), part(part) {}

bool operator< (const gasmix_file &lg, const gasmix_file &rg) {
  return strcmp(lg.name.c_str(), rg.name.c_str()) <= 0;
}

/** \brief функции расчёта средних параметров по методам из
  *   книги "Свойства газов и жидкостей" Рида, Праусница, Шервуда */
namespace rsk_avg {
/** \brief Рассчитать среднюю критическую температуру
  *   простым методом(глава 4.2) */
double dfl_avg_Tk(const parameters_mix &components) {
  double tk = 0.0;
  for (auto const &x : components)
    tk += x.first * x.second.first.T_K;
  return tk;
}
/** \brief Рассчитать среднее критическое давление
  *   по правилу Праусница-Ганна(глава 4.2) */
double dfl_avg_Pk(const parameters_mix &components, double R, double tk) {
  double zk = 0.0, vk = 0.0;
  for (auto const &x : components) {
    zk += x.first * x.second.first.Z_K;
    vk += x.first * x.second.first.V_K;
  }
  return R*zk*tk/vk;
}
/** \brief assert doesn't use it! Have not proof.
 *    Рассчитать среднее значение фактора ацентричности */
// todo: неверный метод(как мне кажется)
//   по книге х используется, а не y
double dfl_avg_acentric(const parameters_mix &components) {
  double w = 0.0;
  for (auto const &x : components)
    w += x.first * x.second.first.acentricfactor;
  return w;
}
/* todo: про критические параметры для разных уравнений состояний
 *   можно почитать в этой же книге, или у Бруссиловского.
 *   По правилу Лоренца-Бертло можно попридумывать функции и для
 *   других моделей. */
/** \brief Рассчитать среднюю критическую температуру по
  *   методу Редлиха-Квонга(двухпараметрическому, глава 4.3) */
double rk2_avg_Tk(const parameters_mix &components) {
  double num = 0.0;
  double dec = 0.0;
  for (auto const &x : components) {
    num += x.first * sqrt(pow(x.second.first.T_K, 2.5) / x.second.first.P_K);
    dec += x.first * x.second.first.T_K / x.second.first.P_K;
  }
  return pow(num, 1.3333) / pow(dec, 0.6667);
}
/** \brief Рассчитать среднее критическое давление по
  *   методу Редлиха-Квонга(двухпараметрическому, глава 4.3) */
double rk2_avg_Pk(const parameters_mix &components) {
  double num = 0.0;
  double dec = 0.0;
  for (auto const &x : components) {
    num += x.first * sqrt(pow(x.second.first.T_K, 2.5) / x.second.first.P_K);
    dec += x.first * x.second.first.T_K / x.second.first.P_K;
  }
  return pow(num, 1.3333) / pow(dec, 1.6667);
}
/** \brief Рассчитать среднее значение фактора ацентричности(глава 4.2) */
double rk2_avg_acentric(const parameters_mix &components) {
  double w = 0.0;
  for (auto const &x : components)
    w += x.first * x.second.first.acentricfactor;
  return w;
}
/** \brief Получить массив средних значений(глава 4.2)
  *   [P_k, T_k, mol, acentric]*/
std::array<double, 4> get_average_params(const parameters_mix &components,
    const model_str &ms) {
  std::array<double, 4> avr_vals = {0.0, 0.0, 0.0, 0.0};
  for (auto const &x : components) {
    // молярная масса
    avr_vals[2] += x.first * x.second.first.molecularmass;
  }
  // тут разграничение по моделям, если руки дойдут
  //   классическая двухпараметрическая модель Редлиха-Квонга
  if (ms.model_type == rg_model_t::REDLICH_KWONG &&
     ms.model_subtype_id == MODEL_SUBTYPE_DEFAULT) {
    avr_vals[0] = rk2_avg_Pk(components);
    avr_vals[1] = rk2_avg_Tk(components);
    avr_vals[3] = rk2_avg_acentric(components);
  } else {
    avr_vals[1] = dfl_avg_Tk(components);
    avr_vals[0] = dfl_avg_Pk(components,
        GAS_CONSTANT / avr_vals[2], avr_vals[1]);
    avr_vals[3] = dfl_avg_acentric(components);
  }
  return avr_vals;
}
}  // average_parameters namespace

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
  GasParameters_mix_dyn *mix = nullptr;
  merror_t err = ERROR_SUCCESS_T;
  if (gpi.const_dyn.components->empty() || mg == nullptr)
    err = init_error.SetError(
        ERROR_INIT_T | ERROR_INIT_NULLP_ST | ERROR_GAS_MIX);
  std::unique_ptr<const_parameters> tmp_cgp = nullptr;
  std::unique_ptr<dyn_parameters> tmp_dgp = nullptr;
  dyn_setup setup = DYNAMIC_SETUP_MASK;
  if (!err) {
    // рассчитать средние критические параметры смеси
    //   как арифметическое среднее её компонентов
    // n.b. это скорее неправильный подход, переделать
    std::array<double, 6> avr_vals = get_average_params(
        *gpi.const_dyn.components);
    // init gasmix const_parameters
    tmp_cgp = std::unique_ptr<const_parameters>(
        const_parameters::Init(GAS_TYPE_MIX, avr_vals[0], avr_vals[1],
        avr_vals[2], avr_vals[3], avr_vals[4], avr_vals[5]));
    if (tmp_cgp == nullptr) {
      err = init_error.SetError(
          ERROR_INIT_T | ERROR_GAS_MIX | ERROR_CALC_GAS_P_ST);
    } else {
      double volume = 0.0;
      assert(0);
      // todo:
      // такс, здесь разбить на 2 этапа:
      //   1) инициализация константных параметров
      //   2) по константным парамертам, получить vpt параметры для смеси
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
        setup &= x.second.setup;
      }
      tmp_dgp = std::unique_ptr<dyn_parameters>(dyn_parameters::Init(setup,
          dgp_tmp[0], dgp_tmp[1], dgp_tmp[2], {volume, gpi.p, gpi.t}));
    }
  }
  if ((!err) && (tmp_dgp != nullptr))
    mix = new GasParameters_mix_dyn({0.0, gpi.p, gpi.t}, *tmp_cgp, *tmp_dgp,
        *gpi.const_dyn.components, mg);
  else
    err = init_error.SetError(
        ERROR_INIT_T | ERROR_CALC_GAS_P_ST | ERROR_GAS_MIX);
  return mix;
}

std::unique_ptr<const_parameters> GasParameters_mix_dyn::GetAverageParams(
    parameters_mix &components) {
  std::unique_ptr<const_parameters> tmp_cgp = nullptr;
  if (!components.empty()) {
    std::array<double, 6> avr_vals = get_average_params(components);
    // init gasmix const_parameters
    if (!(tmp_cgp = std::unique_ptr<const_parameters>(
        const_parameters::Init(GAS_TYPE_MIX, avr_vals[0], avr_vals[1],
        avr_vals[2], avr_vals[3], avr_vals[4], avr_vals[5]))))
      init_error.SetError(ERROR_INIT_T | ERROR_GAS_MIX | ERROR_CALC_GAS_P_ST,
          "Расчёт средних параметров для газовой смеси");
  } else {
    init_error.SetError(ERROR_INIT_T | ERROR_INIT_NULLP_ST | ERROR_GAS_MIX,
        "Инициализация газовой смеси компонентов нет");
  }
  return tmp_cgp;
}

const parameters_mix &GasParameters_mix_dyn::GetComponents() const {
  return components_;
}

void GasParameters_mix_dyn::csetParameters(double v, double p, double t,
    state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume       = v;
  vpte_.pressure     = p;
  vpte_.temperature  = t;
  sph_ = sp;
  // todo: здесь не совсем правильно -
  //   модель выставлена для смеси(скорее всего),
  //   а используется для компонентов отдельно!
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
  }
}
