/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "atherm_common.h"


bool HasGostModelMark(const gas_marks_t m) {
  return m & GAS_NG_GOST_MARK;
}

bool HasGasMixMark(const gas_marks_t m) {
  return m & GAS_MIX_MARK;
}

void AddGostModelMark(gas_marks_t *m) {
  *m = *m | GAS_NG_GOST_MARK;
}

void AddGasMixMark(gas_marks_t *m) {
  *m = *m | GAS_MIX_MARK;
}

rg_model_id::rg_model_id(rg_model_t t, rg_model_subtype subt)
  : type(t), subtype(subt) {}