/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "common.h"
#include "db_connection.h"
#include "ErrorWrap.h"
#include "Logging.h"

#include <ctime>
#include <memory>
#include <string>
#include <vector>


/* CONFIGURATION PARAMETERS (XML or JSON)
 * // MODELS_STR - структура описывающая модель
 * MODELS_STR {
 *   TYPE : INT
 *   SUBTYPE : INT
 *   NAME : STRING
 *   VERSION_MAJOR : INT
 *   VERSION_MINOR : INT
 * }
 *
 * DATABASE_CONFIGURATION {
 *   CLIENT : STRING
 *   // DB_PATH : STRING
 *   NAME : STRING
 *   USERNAME : STRING
 *   PASSWORD : STRING
 *   HOST : STRING
 *   PORT : STRING
 * }
 *
 * PROGRAM_CONFIGURATION:
 * - DEBUG_MODE : BOOL
 * - PSEUDOCRITIC : BOOL
 * - INCLUDE_ISO_20765 : BOOL
 * - LOG_LEVEL : INT
 * - LOG_FILE : STRING
 * - DATABASE : DATABASE_CONFIGURATION
 * // - MODELS : MODELS_STR[] - move to calculation.json
*/

class ConfigurationByFile;


/// структура идентификации модели(уравнения реального газа)
///   параметры прописываются в классе параметров модели
///   методом GetModelStr
struct model_str {
  rg_model_id model_type;
  int32_t vers_major;
  int32_t vers_minor;
  /** \brief информация о модели */
  std::string short_info;

public:
  model_str(rg_model_id ml, int32_t vmaj, int32_t vmin,
      const std::string &info);
};

struct calculation_configuration {
  /** \brief флаг вывода отладочной информации
    * \default true */
  bool is_debug_mode = true;
  /** \brief флаг использования классической модели Редлиха-Квонга
    * \default false */
  bool rk_is_origin_mod = false;
  /** \brief флаг использования модификации Соаве
    *   для модели Редлиха-Квонга
    * \default true */
  bool rk_is_soave_mod = true;
  /** \brief флаг инициализации модели Пенга-Робинсона через
    *   коэфициенты бинарного взаимодействия
    * \default true */
  bool pr_by_binary_coefs = true;
  /** \brief флаг использования 'ISO 20665' поверх 'ГОСТ 30319-3'
    *    для ng_gost модели
    * \default true */
  bool enable_iso_20765 = true;

public:
  bool IsDebug() const;
  bool RK_IsOriginMod() const;
  bool RK_IsSoaveMod() const;
  bool PR_ByBinaryCoefs() const;
  bool EnableISO20765() const;
};

/* сейчас посто заглушка
 * todo: добавить calculation configuration */
struct calculation_info {
  // я хз чё тут надо, но вроде как инпут прописать
  calculation_configuration configuration;
  model_str *model;
  time_t time;
};

/** \brief конфигурация моделей реального газа */
struct models_configuration {
public:
  /** \brief информация о текущем расчёте */
  calculation_configuration calc_cfg;
  /** \brief уровень логирования */
  io_loglvl log_level;
  /** \brief файл логирования */
  std::string log_file;

public:
  models_configuration();
  /** \brief изменить параметр(поле), соответствующий 'param_str',
    *   значение параметра соответствует переданному в строке 'param_value'
    * \param param_str текстовый шаблон поля
    * \param param_value значение параметра */
  merror_t SetConfigurationParameter(const std::string &param_strtpl,
      const std::string &param_value);
};

/** строка для добавления в БД */
struct model_info {
  model_str short_info;
  models_configuration setup_info;
};

/// singleton of state
class ProgramState {
public:
  /** \brief синглетончик инст */
  static ProgramState &Instance();

  /** \brief Загрузить или перезагрузить конфигурацию программы */
  merror_t ResetConfigFile(const std::string &config_file);
  /** \brief Загрузить или перезагрузить параметры газовой смеси */
  merror_t ResetGasmixFile(const std::string &gasmix_file);

  /** \brief Конфигурация из файла была загружена
    * \return true да, false нет */
  bool IsInitialized() const;
  /** \brief Приложение запущено в режиме отладки
    * \return true да, false нет */
  bool IsDebugMode() const;
  /** \brief Приложение работает без подключения к бд
    * \return true да, false нет */
  bool IsDryRunDBConn() const;
  merror_t GetErrorCode() const;
  const models_configuration GetConfiguration() const;
  const calculation_configuration GetCalcConfiguration() const;
  const db_parameters GetDatabaseConfiguration() const;

public:
  /** \brief класс конфигурации расчёта */
  class ProgramConfiguration;

private:
  ProgramState();

private:
  ErrorWrap error_;
  std::unique_ptr<ProgramConfiguration> program_config_;
  // м.б. сразу объект хранить???
  std::string gasmix_file;
};

/// внктренний(nested) класс конфигурации
///   в классе состояния программы
class ProgramState::ProgramConfiguration {
public:
  ProgramConfiguration();
  ProgramConfiguration(const std::string &config_filename);

  merror_t ResetConfigFile(const std::string &new_config_filename);

private:
  /** \brief Установить значения по умолчанию
    *   для возможных параметров */
  void setDefault();
  /** \brief Считать и инициализировать конфигурацию программы */
  void initProgramConfig();
  /** \brief Считать и инициализировать конфигурацию
    *   коннекта к базе данных */
  void initDatabaseConfig();
  /** \brief Считать и инициализировать конфигурацию модели */
  model_str initModelStr();

public:
  ErrorWrap error;
  /** \brief Текущий файл конфигурации */
  std::string config_filename;
  /** \brief Конфигурация программы */
  models_configuration configuration;
  /** \brief Параметры коннекта к БД */
  db_parameters db_parameters_conf;
  /** \brief Набор конфигураций уравнений состояния реального газа */
  std::vector<model_str> models;
  /** \brief По-сути - декоратор над объектом чтения xml(или других форматов)
    *   файлов для конфигурации программы */
  std::unique_ptr<ConfigurationByFile> config_by_file;
  /** \brief чтение файла завершилось успешной загрузкой
    *   конфигуции программы */
  bool is_initialized;
};
#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
