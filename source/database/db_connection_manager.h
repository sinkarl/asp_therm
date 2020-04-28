/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_MANAGER_H_
#define _DATABASE__DB_CONNECTION_MANAGER_H_

#include "db_connection.h"
#include "db_query.h"
#include "ErrorWrap.h"
#include "ThreadWrap.h"

#include <functional>
#include <string>
#include <vector>

#include <stdint.h>


/** \brief Класс инкапсулирующий конечную высокоуровневую операцию с БД
  * \note Определения 'Query' и 'Transaction' в программе условны:
  *   Query - примит обращения к БД, Transaction - связный набор примитивов */
class Transaction {
public:
  class TransactionInfo;

public:
  Transaction(DBConnection *connection);
  // хм, прикольно
  // Transaction (std::mutex &owner_mutex);
  void AddQuery(QuerySmartPtr &&query);
  mstatus_t ExecuteQueries();
  mstatus_t CancelTransaction();

  ErrorWrap GetError() const;
  mstatus_t GetResult() const;
  TransactionInfo GetInfo() const;

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief Указатель на подключение по которому
    *   будет осуществлена транзакция */
  DBConnection *connection_;
  /** \brief Очередь простых запросов, составляющих полную транзакцию */
  QueryContainer queries_;
};

/** \brief Класс инкапсулирующий информацию о транзакции - лог, результат
  * \note Не доделан */
class Transaction::TransactionInfo {
  friend class Transaction;
// date and time +
// info
public:
  std::string GetInfo();

private:
  TransactionInfo();

private:
  std::string info_;
};


/** \brief Класс взаимодействия с БД, предоставляет API
  *   на все допустимые операции */
class DBConnectionManager {
public:
  DBConnectionManager();
  // API DB
  mstatus_t CheckConnection();
  // static const std::vector<std::string> &GetJSONKeys();
  /** \brief Попробовать законектится к БД */
  mstatus_t ResetConnectionParameters(const db_parameters &parameters);
  /** \brief Проверка существования таблицы */
  bool IsTableExist(db_table dt);
  /** \brief Создать таблицу */
  mstatus_t CreateTable(db_table dt);

  /* insert operations */
  /** \brief Сохранить в БД строку model_info */
  mstatus_t SaveModelInfo(model_info &mi);
  /** \brief Сохранить в БД строку calculation_info */
  mstatus_t SaveCalculationInfo(calculation_info &ci);
  /** \brief Сохранить в БД строку calculation_info */
  mstatus_t SaveCalculationStateInfo(std::vector<calculation_state_log> &csi);

  /* select operations */
  /** \brief Вытащить из БД строки model_info по 'where' условиям */
  mstatus_t SelectModelInfo(model_info &where, std::vector<model_info> *res);

  /* todo: select, update methods */

  /* rename method to GetError */
  merror_t GetErrorCode();
  mstatus_t GetStatus();
  /* remove this(GetErrorMessage), add LogIt */
  std::string GetErrorMessage();

private:
  class DBConnectionCreator;

private:
  /** \brief
    *
  */
  template <class DataT, class OutT, class SetupQueryF>
  mstatus_t exec_wrap(DataT data, OutT *res, SetupQueryF setup_m) {
    if (status_ == STATUS_DEFAULT)
      status_ = CheckConnection();
    if (db_connection_ && is_status_aval(status_)) {
      Transaction tr(db_connection_.get());
      tr.AddQuery(QuerySmartPtr(
          new DBQuerySetupConnection(db_connection_.get())));
      // добавить специализированные запросы
      //   true - если вызов функции успешен
      std::invoke(setup_m, *this, &tr, data, res);
      tr.AddQuery(QuerySmartPtr(
          new DBQueryCloseConnection(db_connection_.get())));
      status_ = tryExecuteTransaction(tr);
    } else {
      error_.SetError(ERROR_DB_CONNECTION, "Не удалось установить "
          "соединение для БД: " + parameters_.GetInfo());
      status_ = STATUS_HAVE_ERROR;
    }
    return status_;
  }

  /** \brief Проинициализировать соединение с БД */
  void initDBConnection();

  /* добавить в транзакцию соответствующий запрос */
  /** \brief  */
  void isTableExist(Transaction *tr, db_table dt, bool *is_exists);
  /** \brief  */
  void createTable(Transaction *tr, db_table dt, void *);
  /** \brief  */
  void saveRow(Transaction *tr, const db_query_insert_setup &qi, void *);

  /** \brief провести транзакцию tr из собраных запросов(строк) */
  [[nodiscard]]
  mstatus_t tryExecuteTransaction(Transaction &tr);

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief Мьютекс на подключение к БД */
  SharedMutex connect_init_lock_;
  /** \brief Параметры текущего подключения к БД */
  db_parameters parameters_;
  /** \brief Указатель иницианилизированное подключение */
  std::unique_ptr<DBConnection> db_connection_;
};


/** \brief Закрытый класс создания соединений с БД */
class DBConnectionManager::DBConnectionCreator {
  friend class DBConnectionManager;

private:
  DBConnectionCreator();

  DBConnection *InitDBConnection(const db_parameters &parameters);
};

#endif  // !_DATABASE__DB_CONNECTION_MANAGER_H_
