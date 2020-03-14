/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODEL_REDLICH_KWONG_SOAVE_H_
#define _CORE__MODELS__MODEL_REDLICH_KWONG_SOAVE_H_

#include "common.h"
#include "gasmix_init.h"
#include "model_general.h"

#include <array>
#include <memory>

#ifdef RKS_UNITTEST
class Redlich_Kwong_Soave: public modelGeneral {
#else
/** \brief Модификация Соаве уравнения состояния
  *   Редлиха-Квонга */
class Redlich_Kwong_Soave final: public modelGeneral {
#endif  // RKS_UNITTEST
public:
  static Redlich_Kwong_Soave *Init(const model_input &mi);

  model_str GetModelShortInfo() const override;

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  double InitVolume(double p, double t,
      const const_parameters &cp) override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;

  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

protected:
  Redlich_Kwong_Soave(const model_input &mi);

  /* интерпретация из книги Рида, Праусница, Шервуда
   *   я с ней не разобрался просто */
  /* удоли */
  /** \brief Расчитать F параметр для модифмкации Соаве */
  static double calculate_F(double t, const const_parameters &cp);
  /** \brief Расчитать F параметр для модифмкации Соаве */
  static double calculate_F(double t, double wf, const const_parameters &cp);
  /** \brief Расчитать F параметр для модифмкации Соаве */
  static std::pair<double, double> calculate_ab_coefs(
      const const_parameters &cp);
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   по параметрам газа parameters_ */
  void set_model_coef();
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   по переданным параметрам cp  */
  void set_model_coef(const const_parameters &cp);
  /* классический подход из книг Бруссиловского, алсо см. Публикации Соаве */
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   для газовой смеси по методу Соаве-Редлиха-Квонга */
  void gasmix_model_coefs(const model_input &mi);
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   для газовой смеси по методу Соаве-Редлиха-Квонга через
    *   параметр F - обобщённый подход для модели
    *   РК из Рида, Праусница, Шервуда */
  void gasmix_model_coefs_rps(const model_input &mi);


protected:
  /** \brief набор константных параметров компонента смеси
    *   зависящих от фактрора ацентричности и параметров в газа
    *   критической точке - P_k, T_k, V_k */
  struct const_rks_vals {
    /// SRK: 0.480 + 1.574w - 0.176w^2
    std::vector<double> fw_i;  // cap: n
    /// SRK: xi*xj*(1.0 - kiy) * sqrt(Tci*Tcj / (Pci*Pcj))
    std::vector<std::vector<double>> fsqrt_tp_ij;  // cap: n^2
    /// SRK: SUM xi*Tci/Pci
    double ftp_sum;

    /** \brief Для случая смеси газов инициализировать набор
      *   постоянных величин компонентов */
    void set_vals(const parameters_mix *components);
  } const_rks_vals_;

  double model_coef_a_,
         model_coef_b_;
};
#endif  // !_CORE__MODELS__MODEL_REDLICH_KWONG_H_
