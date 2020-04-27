/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_QUERY_H_
#define _DATABASE__DB_QUERY_H_

#include "db_defines.h"
#include "db_queries_setup.h"
#include "gas_description.h"
#include "ErrorWrap.h"

#include <memory>
#include <string>
#include <vector>


class DBConnection;
struct db_table_create_setup;

/** \brief абстрактный класс запросов */
class DBQuery {
public:
  bool IsPerformed() const { return is_performed_; }

  void LogError();

  /** \brief Обёртка над функцией исполнения команды */
  virtual mstatus_t Execute();
  virtual void unExecute() = 0;
  virtual ~DBQuery();

protected:
  DBQuery(DBConnection *db_ptr);
  /** \brief Функция исполнения команды */
  virtual mstatus_t exec() = 0;
  virtual std::string q_info() = 0;

protected:
  // std::string query_body_;
  mstatus_t status_;
  /** \brief Указатель на подключение к БД */
  DBConnection *db_ptr_;
  bool is_performed_;
};
typedef std::shared_ptr<DBQuery> QuerySmartPtr;
typedef std::vector<QuerySmartPtr> QueryContainer;

/** \brief Запрос установить соединение с бд */
class DBQuerySetupConnection: public DBQuery {
public:
  DBQuerySetupConnection(DBConnection *db_ptr);
  /** \brief отключиться от бд */
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;
};

/** \brief Запрос отключения от бд */
class DBQueryCloseConnection: public DBQuery {
public:
  DBQueryCloseConnection(DBConnection *db_ptr);
  mstatus_t Execute() override;
  /** \brief не делать ничего */
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;
};

/** \brief Запрос проверки существования таблицы в бд */
class DBQueryIsTableExists: public DBQuery {
public:
  DBQueryIsTableExists(DBConnection *db_ptr, db_table dt, bool &is_exists);
  /** \brief не делать ничего */
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;

private:
  db_table table_;
  bool &is_exists_;
};

/** \brief Запрос создания таблицы в бд */
class DBQueryCreateTable: public DBQuery {
public:
  DBQueryCreateTable(DBConnection *db_ptr,
      const db_table_create_setup &create_setup);
 // mstatus_t Execute() override;
  /** \brief обычный rollback создания таблицы */
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;

private:
  const db_table_create_setup &create_setup;
};

/** \brief Запрос обновления формата таблицы в бд
  * \note Обновление таблицы БД */
class DBQueryUpdateTable: public DBQuery {
public:
  DBQueryUpdateTable(DBConnection *db_ptr,
      const db_table_create_setup &table_setup);
  mstatus_t Execute() override;
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;
};

/** \brief Запрос на добавление строки */
class DBQueryInsertRows: public DBQuery {
public:
  DBQueryInsertRows(DBConnection *db_ptr,
      const db_query_insert_setup &insert_setup);
 // mstatus_t Execute() override;
  void unExecute() override;

protected:
  mstatus_t exec() override;
  std::string q_info() override;

private:
  const db_query_insert_setup &insert_setup;
};

#endif  // !_DATABASE__DB_QUERY_H_
