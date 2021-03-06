/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * configuration_by_file *
 *   В файле описан шаблон класса, предоставляющего функционал
 * инициализации конфигурации программы из текстового файла.
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_
#define _CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_

#include "asp_db/db_connection.h"
#include "asp_utils/ErrorWrap.h"
#include "configuration_strtpl.h"
#include "file_structs.h"
#include "models_configurations.h"

#include <memory>
#include <optional>
#include <set>
#include <string>

/** \brief Класс загрузки файла конфигурации программы */
template <template <class config_node> class ConfigReader>
class ConfigurationByFile : public BaseObject {
 public:
  ConfigurationByFile(const ConfigurationByFile&) = delete;
  ConfigurationByFile& operator=(const ConfigurationByFile&) = delete;

  static ConfigurationByFile* Init(const std::string& filename) {
    ConfigReader<config_node>* cr = ConfigReader<config_node>::Init(filename);
    if (cr == nullptr)
      return nullptr;
    return new ConfigurationByFile(cr);
  }

  bool HasConfiguration() const { return configuration_.has_value(); }
  auto GetConfiguration() const { return configuration_; }

  auto GetDBConfiguration() const { return db_parameters_; }

  const ErrorWrap& GetErrorWrap() const { return error_; }

 private:
  explicit ConfigurationByFile(ConfigReader<config_node>* config_doc)
      : BaseObject(STATUS_DEFAULT), config_doc_(config_doc) {
    if (config_doc) {
      init_parameters();
    } else {
      error_.SetError(ERROR_INIT_T,
                      "Инициализация XMLReader для файла конфигурации");
      error_.LogIt();
    }
  }

  /** \brief Инициализировать общую конфигурацию программы */
  merror_t init_parameters() {
    merror_t error = ERROR_SUCCESS_T;
    std::vector<std::string> param_path(1);
    std::string tmp_str;
    configuration_ = program_configuration{};
    for (const auto& param : config_params) {
      param_path[0] = param;
      if (param == STRTPL_CONFIG_DATABASE) {
        if ((error = init_dbparameters()))
          break;
      } else {
        config_doc_->GetValueByPath(param_path, &tmp_str);
        error = configuration_.value().SetConfigurationParameter(param, tmp_str);
      }
      if (error) {
        error_.SetError(error,
                        "Error during configfile reading: " + param_path[0]);
        error_.LogIt();
        break;
      }
    }
    return error;
  }

  /** \brief Инициализировать конфигурацию подключения к БД */
  merror_t init_dbparameters() {
    merror_t error = ERROR_SUCCESS_T;
    std::vector<std::string> param_path =
        std::vector<std::string>{STRTPL_CONFIG_DATABASE, ""};
    std::string tmp_str = "";
    db_parameters_ = asp_db::db_parameters{};
    for (const auto& param : config_database) {
      param_path[1] = param;
      config_doc_->GetValueByPath(param_path, &tmp_str);
      error = set_db_parameter(&db_parameters_.value(), param, tmp_str);
      if (error) {
        error_.SetError(
            error,
            "Ошибка обработки параметра файла конфигурации БД: " + param_path[1]);
        error_.LogIt();
        db_parameters_ = std::nullopt;
        break;
      }
    }
    return error;
  }

 private:
  std::unique_ptr<ConfigReader<config_node>> config_doc_;
  std::optional<program_configuration> configuration_{std::nullopt};
  std::optional<asp_db::db_parameters> db_parameters_{std::nullopt};

 private:
  /** \brief строковые идентификаторы параметров
   *   конфигурации программы */
  static std::set<std::string> config_params;
  /** \brief строковые идентификаторы параметров
   *   конфигурации подключения к БД */
  static std::set<std::string> config_database;
};

/* todo: убрать это и переделать инициализацию по xml */
template <template <class config_node> class ConfigReader>
std::set<std::string> ConfigurationByFile<ConfigReader>::config_params =
    std::set<std::string>{
        STRTPL_CONFIG_DEBUG_MODE,        STRTPL_CONFIG_RK_ORIG_MOD,
        STRTPL_CONFIG_RK_SOAVE_MOD,      STRTPL_CONFIG_PR_BINARYCOEFS,
        STRTPL_CONFIG_INCLUDE_ISO_20765, STRTPL_CONFIG_LOG_LEVEL,
        STRTPL_CONFIG_LOG_FILE,          STRTPL_CONFIG_DATABASE};
template <template <class config_node> class ConfigReader>
std::set<std::string> ConfigurationByFile<ConfigReader>::config_database =
    std::set<std::string>{STRTPL_CONFIG_DB_DRY_RUN,  STRTPL_CONFIG_DB_CLIENT,
                          STRTPL_CONFIG_DB_NAME,     STRTPL_CONFIG_DB_USERNAME,
                          STRTPL_CONFIG_DB_PASSWORD, STRTPL_CONFIG_DB_HOST,
                          STRTPL_CONFIG_DB_PORT};

#endif  // !_CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_
